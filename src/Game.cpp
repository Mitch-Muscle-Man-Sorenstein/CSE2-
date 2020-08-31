#include "Game.h"

#include <stddef.h>
#include <string>
#include <iostream>

#include "WindowsWrapper.h"

#include "Backends/Misc.h"
#include "ArmsItem.h"
#include "Back.h"
#include "Boss.h"
#include "BossLife.h"
#include "BulHit.h"
#include "Bullet.h"
#include "Caret.h"
#include "Config.h"
#include "CommonDefines.h"
#include "Draw.h"
#include "Ending.h"
#include "Escape.h"
#include "Fade.h"
#include "Flags.h"
#include "Flash.h"
#include "Frame.h"
#include "Generic.h"
#include "GenericLoad.h"
#include "KeyControl.h"
#include "Main.h"
#include "Map.h"
#include "MapName.h"
#include "MiniMap.h"
#include "Music.h"
#include "MyChar.h"
#include "MycHit.h"
#include "MycParam.h"
#include "NpChar.h"
#include "NpcHit.h"
#include "NpcTbl.h"
#include "Profile.h"
#include "Random.h"
#include "SelStage.h"
#include "Shoot.h"
#include "Sound.h"
#include "Stage.h"
#include "Star.h"
#include "TextScr.h"
#include "Title.h"
#include "ValueView.h"
#include "Filesystem.h"

GameDifficulty g_GameDifficulty;

int g_GameFlags;
int gCounter;

BOOL LoadAssets()
{
	LoadSounds();
	if (!(LoadSurfaces() && LoadStageTable() && LoadMusicTable() && LoadNpcTable()))
		return FALSE;
	return TRUE;
}

BOOL SetOriginalGraphics(BOOL use_original)
{
	if (gUseOriginalGraphics != use_original)
	{
		gUseOriginalGraphics = use_original;
		if (!LoadSurfaces())
			return FALSE;
	}
	return TRUE;
}

BOOL SetMod(std::string mod)
{
	if (Filesystem_GetMod() != mod)
	{
		//Set mod and reload tables
		Filesystem_SetMod(mod);
		if (!LoadAssets())
			return FALSE;
	}
	return TRUE;
}

int Random(int min, int max)
{
	const int range = max - min + 1;
	return (msvc_rand() % range) + min;
}

void PutNumber4(int x, int y, int value, BOOL bZero)
{
	// Define rects
	RECT rcClient = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

	RECT rect[10] = {
		{0, 56, 8, 64},
		{8, 56, 16, 64},
		{16, 56, 24, 64},
		{24, 56, 32, 64},
		{32, 56, 40, 64},
		{40, 56, 48, 64},
		{48, 56, 56, 64},
		{56, 56, 64, 64},
		{64, 56, 72, 64},
		{72, 56, 80, 64},
	};

	// Digits
	int tbl[4] = {1000, 100, 10, 1};

	int a;
	int sw;
	int offset;

	// Limit value
	if (value > 9999)
		value = 9999;

	// Go through number and draw digits
	offset = 0;
	sw = 0;
	while (offset < 4)
	{
		// Get the digit that this is
		a = 0;

		while (value >= tbl[offset])
		{
			value -= tbl[offset];
			++a;
			++sw;
		}

		// Draw digit
		if ((bZero && offset == 2) || sw != 0 || offset == 3)
			PutBitmap3(&rcClient, x + 8 * offset, y, &rect[a], SURFACE_ID_TEXT_BOX);

		// Go to next digit
		++offset;
	}
}

BOOL TransitionWait()
{
	ChangeMusic(MUS_SILENCE);
	for (int i = 0; i < 8; i++)
	{
		CortBox(&grcGame, 0x000000);
		PutFramePerSecound();
		if (!Flip_SystemTask())
			return FALSE;
	}
	return TRUE;
}

static int ModeOpening(void)
{
	int frame_x;
	int frame_y;
	unsigned int wait;

	//Clear mod and use selected graphics
	SetMod("");
	SetOriginalGraphics(gConfig.original_graphics);

	//Initialize game state
	InitNpChar();
	InitCaret();
	InitStar();
	InitFade();
	InitFlash();
	InitBossLife();
	CutNoise();
	g_GameFlags = 3;
	
	//Goto intro stage with no music
	ChangeMusic(MUS_SILENCE);
	if (!TransferStage(72, 100, 3, 3))
	{
		Backend_ShowMessageBox("Error", "Failed to load stage");
		return 0;
	}
	SetFrameTargetMyChar(16);
	SetFadeMask();

	//Reset cliprect and flags
	grcGame.left = 0;
	grcGame.top = 0;
	grcGame.right = WINDOW_WIDTH;
	grcGame.bottom = WINDOW_HEIGHT;

	wait = 0;
	while (wait < 500)
	{
		// Increase timer
		++wait;

		// Get pressed keys
		GetTrg();

		// Skip intro if OK is pressed
		if (gKey & gKeyOk)
			break;

		// Update everything
		ActNpChar();
		ActBossChar();
		ActBack();
		ResetMyCharFlag();
		HitMyCharMap();
		HitMyCharNpChar();
		HitMyCharBoss();
		HitNpCharMap();
		HitBossMap();
		HitBossBullet();
		ActCaret();
		MoveFrame3();
		ProcFade();

		// Draw everything
		CortBox(&grcFull, 0x000000);

		GetFramePosition(&frame_x, &frame_y);
		PutBack(frame_x, frame_y);
		PutStage_Back(frame_x, frame_y);
		PutBossChar(frame_x, frame_y);
		PutNpChar(frame_x, frame_y);
		PutMapDataVector(frame_x, frame_y);
		PutStage_Front(frame_x, frame_y);
		PutFront(frame_x, frame_y);
		PutCaret(frame_x, frame_y);
		PutFade();

		// Update Text Script
		switch (TextScriptProc())
		{
			case enum_ESCRETURN_exit:
				return 0;

			case enum_ESCRETURN_restart:
				return 1;
		}

		PutMapName(FALSE);
		PutTextScript();
		PutFramePerSecound();

		if (!Flip_SystemTask())
			return 0;

		++gCounter;
	}

	if (!TransitionWait())
		return 0;
	return 2;
}

//Title menu manager
struct MenuEntry
{
	BOOL flag;
	BOOL *condition;
};

class MenuManager
{
	private:
		const MenuEntry *menu;
		int pos;
		
	public:
		//Constructors
		MenuManager() {}
		MenuManager(const MenuEntry *_menu) : menu(_menu) {}
		
		//Menu
		void SetMenu(const MenuEntry *_menu) { menu = _menu; SetPos(0); }
		const MenuEntry *GetMenu() { return menu; }
		const MenuEntry *GetEntry() { return &menu[pos]; }
		
		//Positioning
		void SetPos(int _pos) { pos = _pos; while (menu[pos].condition && !*menu[pos].condition) pos++; }
		int GetPos() { return pos; }
		
		void IncPos()
		{
			do
			{
				if (!menu[++pos].flag)
					pos = 0;
			} while (menu[pos].condition && !*menu[pos].condition);
		}
		
		void DecPos()
		{
			do
			{
				if (--pos < 0)
				{
					while (menu[pos + 1].flag)
						pos++;
				}
			} while (menu[pos].condition && !*menu[pos].condition);
		}
		
		void MoveCursor()
		{
			if (gKeyTrg & (gKeyUp | gKeyDown))
			{
				PlaySoundObject(1, 1);
				if (gKeyTrg & gKeyUp)
					DecPos();
				if (gKeyTrg & gKeyDown)
					IncPos();
			}
		}
};

//Title saves
PROFILE saves[3];
int saveBase;
BOOL hasSaves;

void LoadSaves(int base)
{
	saveBase = base;
	GetProfile(base + 0, &saves[0]);
	GetProfile(base + 1, &saves[1]);
	GetProfile(base + 2, &saves[2]);
	hasSaves = saves[0].flag || saves[1].flag || saves[2].flag;
}

//Title loop
static int ModeTitle(void)
{
	//Set rects
	static RECT rcTitle = {0, 0, 210, 49};
	
	//Set colours
	static const unsigned long text_white_col = 0xFFFFFF;
	static const unsigned long text_grey_col = 0x7F7F7F;
	
	//Quote cursor
	static RECT rcQuote[4] = {
		{0,  16, 16, 32},
		{16, 16, 32, 32},
		{0,  16, 16, 32},
		{32, 16, 48, 32},
	};
	
	RECT *rcCursor = rcQuote;
	
	int anime = 0;
	
	//Reset game stuff
	CutNoise();
	
	//Clear mod and use new graphics
	std::string mod = "";
	SetMod("");
	SetOriginalGraphics(FALSE);
	
	//Initialize background
	InitBack("bkMoon", 8);
	
	//Play music
	if (g_GameSeason != GS_Pixel || !LoadMusic("ika"))
		LoadMusic("curly");
	PlayMusic();
	
	//Reset cliprect
	grcGame.left = 0;
	grcGame.top = 0;
	grcGame.right = WINDOW_WIDTH;
	grcGame.bottom = WINDOW_HEIGHT;
	
	//Initialize title
	enum TitleMode
	{
		TM_Title,
		TM_Menu,
		TM_Save,
		TM_New,
		TM_Delete,
		TM_DeleteConfirm,
		TM_Quit,
		TM_Play,
	};
	TitleMode mode = TM_Title;
	
	//Main menu
	static BOOL hasCurly = TRUE;
	
	enum MMID
	{
		MMID_StoryMode,
		MMID_CurlyStory,
		MMID_Challenges,
		MMID_GameOptions,
		MMID_Quit,
	};
	
	static const MenuEntry main_menu[] = {
		{TRUE, nullptr},
		{TRUE, &hasCurly},
		{TRUE, nullptr},
		{TRUE, nullptr},
		{TRUE, nullptr},
		{FALSE, nullptr}
	};
	
	MenuManager main_menu_manager(main_menu);
	
	//Save menu
	enum SMID
	{
		SMID_Save1,
		SMID_Save2,
		SMID_Save3,
		SMID_Delete,
		SMID_Back,
	};
	
	static const MenuEntry save_menu[] = {
		{TRUE, nullptr},
		{TRUE, nullptr},
		{TRUE, nullptr},
		{TRUE, &hasSaves},
		{TRUE, nullptr},
		{FALSE, nullptr}
	};
	
	MenuManager save_menu_manager(save_menu);
	
	//New save menu
	enum NMID
	{
		NMID_Difficulty,
		NMID_Play,
		NMID_Back,
	};
	
	static const MenuEntry new_menu[] = {
		{TRUE, nullptr},
		{TRUE, nullptr},
		{TRUE, nullptr},
		{FALSE, nullptr}
	};
	
	MenuManager new_menu_manager(new_menu);
	
	//Delete save menu
	enum DMID
	{
		DMID_Save1,
		DMID_Save2,
		DMID_Save3,
		DMID_Cancel,
	};
	
	MenuEntry delete_menu[] = {
		{TRUE, &saves[0].flag},
		{TRUE, &saves[1].flag},
		{TRUE, &saves[2].flag},
		{TRUE, nullptr},
		{FALSE, nullptr}
	};
	
	MenuManager delete_menu_manager(delete_menu);
	
	//Delete confirm save menu
	enum DCMID
	{
		DCMID_Yes,
		DCMID_No,
	};
	
	static const MenuEntry delete_confirm_menu[] = {
		{TRUE, nullptr},
		{TRUE, nullptr},
		{FALSE, nullptr}
	};
	
	MenuManager delete_confirm_menu_manager(delete_confirm_menu);
	
	//Quit menu
	enum QMID
	{
		QMID_Yes,
		QMID_No,
	};
	
	static const MenuEntry quit_menu[] = {
		{TRUE, nullptr},
		{TRUE, nullptr},
		{FALSE, nullptr}
	};
	
	MenuManager quit_menu_manager(quit_menu);
	
	//Start loop
	while (mode != TM_Play)
	{
		//Get pressed keys
		GetTrg();
		
		//Draw background
		ActBack();
		PutBack(0, 0);
		
		//Animate Quote cursor
		if (++anime >= 32)
			anime = 0;
		int frame = anime / 8;
		
		//Process title
		switch (mode)
		{
			case TM_Title:
			{
				//Go to next screen when Ok is pressed
				if (gKeyTrg & gKeyOk)
				{
					PlaySoundObject(1, 1);
					mode = TM_Menu;
					main_menu_manager.SetPos(0);
				}
				
				//Draw "Cave Story" logo
				PutBitmap3(&grcGame, (WINDOW_WIDTH - rcTitle.right) / 2, 53, &rcTitle, SURFACE_ID_TITLE);
				
				//Draw text
				Title_PutCenterText(WINDOW_WIDTH / 2, 140, "Press <Z> to begin", GetCortBoxColor(0xFFFFFF));
				Title_PutCenterText(WINDOW_WIDTH / 2, 155, "(navigate with Up Arrow and", GetCortBoxColor(0xCCAAAA));
				Title_PutCenterText(WINDOW_WIDTH / 2, 164, "Down Arrow on the menu)", GetCortBoxColor(0xCCAAAA));
				Title_PutCenterText(WINDOW_WIDTH / 2, 216, "@2020 MITCH \"MUSCLE MAN\" SORENSTEIN", GetCortBoxColor(0xFFFFFF));
				break;
			}
			case TM_Menu:
			{
				//Go to quit menu if escape is pressed
				if (gKeyTrg & KEY_ESCAPE)
				{
					mode = TM_Quit;
					quit_menu_manager.SetPos(0);
				}
				
				//Move cursor
				main_menu_manager.MoveCursor();
				
				//Handle option when Ok is pressed
				if (gKeyTrg & gKeyOk)
				{
					PlaySoundObject(1, 1);
					switch (main_menu_manager.GetPos())
					{
						case MMID_StoryMode:
							mode = TM_Save;
							LoadSaves(0);
							save_menu_manager.SetPos(SMID_Save1);
							break;
						case MMID_CurlyStory:
							mode = TM_Save;
							mod = "CurlyStory";
							LoadSaves(10);
							save_menu_manager.SetPos(SMID_Save1);
							break;
						case MMID_Quit:
							mode = TM_Quit;
							quit_menu_manager.SetPos(QMID_Yes);
							break;
					}
				}
				
				//Draw "Cave Story" logo
				PutBitmap3(&grcGame, (WINDOW_WIDTH - rcTitle.right) / 2, 53, &rcTitle, SURFACE_ID_TITLE);
				
				//Draw backing box
				RECT rcBox = {(WINDOW_WIDTH - 102) / 2, 132, (WINDOW_WIDTH + 102) / 2, 214};
				Title_PutBox(&rcBox, FALSE);
				
				//Draw Quote cursor
				PutBitmap3(&grcGame, (WINDOW_WIDTH / 2) - 45, 138 + 14 * main_menu_manager.GetPos(), &rcCursor[frame], SURFACE_ID_MY_CHAR);
				
				//Draw options
				static const char *names[] = {
					"Story Mode",
					"Curly Story",
					"Challenges...",
					"Game Options",
					"Quit"
				};
				for (int i = 0;; i++)
				{
					const MenuEntry *entry = &main_menu[i];
					if (entry->flag)
						PutText((WINDOW_WIDTH / 2) - 26, 142 + 14 * i, (entry->condition == nullptr || *entry->condition) ? names[i] : "      ? ? ?", (entry->condition == nullptr || *entry->condition) ? text_white_col : text_grey_col);
					else
						break;
				}
				break;
			}
			case TM_Save:
			{
				//Move cursor
				save_menu_manager.MoveCursor();
				
				//Handle option when Ok is pressed
				if (gKeyTrg & gKeyOk)
				{
					PlaySoundObject(1, 1);
					int id;
					switch (id = save_menu_manager.GetPos())
					{
						case SMID_Save1:
						case SMID_Save2:
						case SMID_Save3:
							//Remember this specific profile id
							gProfileId = saveBase + id - SMID_Save1;
							
							if (saves[id - SMID_Save1].flag)
							{
								//Play save (it exists)
								mode = TM_Play;
							}
							else
							{
								//Enter new save menu
								mode = TM_New;
								g_GameDifficulty = GD_Easy;
								new_menu_manager.SetPos(NMID_Difficulty);
							}
							break;
						case SMID_Delete:
							mode = TM_Delete;
							delete_menu_manager.SetPos(DMID_Save1);
							break;
						case SMID_Back:
							mode = TM_Menu;
							mod = "";
							break;
					}
				}
				
				//Draw saves
				for (int i = 0; i < 3; i++)
					Title_PutSave(saves[i].flag ? &saves[i] : nullptr, 20 + 50 * i, save_menu_manager.GetPos() == (SMID_Save1 + i));
				
				//Draw text boxes
				if (hasSaves)
					Title_PutCenterTextBox(WINDOW_WIDTH / 2, 178, "Delete a Save", save_menu_manager.GetPos() == SMID_Delete);
				Title_PutCenterTextBox(WINDOW_WIDTH / 2, 208, "Back", save_menu_manager.GetPos() == SMID_Back);
				break;
			}
			case TM_New:
			{
				//Move cursor
				new_menu_manager.MoveCursor();
				
				if (new_menu_manager.GetPos() == NMID_Difficulty)
				{
					//Handle changing difficulty
					if (gKeyTrg & (gKeyLeft | gKeyRight))
					{
						PlaySoundObject(1, 1);
						if (gKeyTrg & gKeyLeft)
							g_GameDifficulty = (GameDifficulty)((g_GameDifficulty <= 0) ? GD_Hard : (g_GameDifficulty - 1));
						if (gKeyTrg & gKeyRight)
							g_GameDifficulty = (GameDifficulty)((g_GameDifficulty + 1) % 3);
					}
				}
				else
				{
					//Handle option when Ok is pressed
					if (gKeyTrg & gKeyOk)
					{
						PlaySoundObject(1, 1);
						switch (new_menu_manager.GetPos())
						{
							case NMID_Play:
								mode = TM_Play;
								break;
							case NMID_Back:
								mode = TM_Menu;
								mod = "";
								break;
						}
					}
				}
				
				//Draw text boxes
				Title_PutTextBox2((WINDOW_WIDTH / 2) - 74, 96, "Difficulty Level:", new_menu_manager.GetPos() == NMID_Difficulty);
				Title_PutTextBox2((WINDOW_WIDTH / 2) - 74, 149, "Start Game", new_menu_manager.GetPos() == NMID_Play);
				Title_PutTextBox2((WINDOW_WIDTH / 2) - 74, 175, (g_GameDifficulty == GD_Hard) ? "I'm too afraid!" : "Cancel", new_menu_manager.GetPos() == NMID_Back);
				
				//Draw difficulty
				static const char *difficulties[] = {
					"   Easy   ",
					" Original ",
					"   Hard   ",
				};
				static const char *difficulty_labels[] = {
					"Best choice for first-time players.",
					"For players familiar with Cave Story.",
					"Not recommended for first-time players."
				};
				RECT rcBox = {(WINDOW_WIDTH / 2) + 22, 88, (WINDOW_WIDTH / 2) + 82, 112};
				Title_PutBox(&rcBox, new_menu_manager.GetPos() == NMID_Difficulty);
				PutText((WINDOW_WIDTH / 2) + 30, 96, difficulties[g_GameDifficulty], text_white_col);
				PutText((WINDOW_WIDTH / 2) - 82, 117, difficulty_labels[g_GameDifficulty], text_white_col);
				break;
			}
			case TM_Delete:
			{
				//Move cursor
				delete_menu_manager.MoveCursor();
				
				//Handle option when Ok is pressed
				if (gKeyTrg & gKeyOk)
				{
					PlaySoundObject(1, 1);
					int id;
					switch (id = delete_menu_manager.GetPos())
					{
						case DMID_Save1:
						case DMID_Save2:
						case DMID_Save3:
							mode = TM_DeleteConfirm;
							gProfileId = saveBase + id - DMID_Save1;
							delete_confirm_menu_manager.SetPos(DCMID_No);
							break;
						case DMID_Cancel:
							mode = TM_Menu;
							mod = "";
							break;
					}
				}
				
				//Draw saves
				for (int i = 0; i < 3; i++)
					Title_PutSave(saves[i].flag ? &saves[i] : nullptr, 50 + 50 * i, delete_menu_manager.GetPos() == (DMID_Save1 + i));
				
				//Draw text boxes
				Title_PutTextBox2((WINDOW_WIDTH / 2) - 26, 208, "Cancel", delete_menu_manager.GetPos() == DMID_Cancel);
				
				//Draw label
				PutText(10, 10, "Choose the save you would like to delete.", text_white_col);
				break;
			}
			case TM_DeleteConfirm:
			{
				//Move cursor
				delete_confirm_menu_manager.MoveCursor();
				
				//Handle option when Ok is pressed
				if (gKeyTrg & gKeyOk)
				{
					PlaySoundObject(1, 1);
					switch (delete_confirm_menu_manager.GetPos())
					{
						case DCMID_Yes:
							DeleteProfile(gProfileId);
							saves[gProfileId - saveBase].flag = false;
					//Fallthrough
						case DCMID_No:
							mode = TM_Save;
							save_menu_manager.SetPos(SMID_Save1);
							break;
					}
				}
				
				//Draw text boxes
				Title_PutCenterTextBox(WINDOW_WIDTH / 2, 148, "Yes", delete_confirm_menu_manager.GetPos() == DCMID_Yes);
				Title_PutCenterTextBox(WINDOW_WIDTH / 2, 178, "No", delete_confirm_menu_manager.GetPos() == DCMID_No);
				
				//Draw label
				PutText(30, 75, "Are you sure you want to delete this save data?", text_white_col);
				break;
			}
			case TM_Quit:
			{
				//Move cursor
				quit_menu_manager.MoveCursor();
				
				//Quit if escape is pressed
				if (gKeyTrg & KEY_ESCAPE)
					return 0;
				
				//Handle option when Ok is pressed
				if (gKeyTrg & gKeyOk)
				{
					PlaySoundObject(1, 1);
					switch (quit_menu_manager.GetPos())
					{
						case QMID_Yes:
							return 0;
						case QMID_No:
							mode = TM_Menu;
							main_menu_manager.SetPos(MMID_StoryMode);
							break;
					}
				}
				
				//Draw text and boxes
				Title_PutCenterText(WINDOW_WIDTH / 2, 120, "Are you sure you want to quit?", text_white_col);
				Title_PutCenterTextBox(WINDOW_WIDTH / 2, 148, "Yes", quit_menu_manager.GetPos() == QMID_Yes);
				Title_PutCenterTextBox(WINDOW_WIDTH / 2, 178, "No", quit_menu_manager.GetPos() == QMID_No);
				break;
			}
			case TM_Play:
			{
				break;
			}
		};
		
		PutFramePerSecound();
		if (!Flip_SystemTask())
			return 0;
		SetMod(mod);
	}

	ChangeMusic(MUS_SILENCE);
	return 3;
}

static int ModeAction(void)
{
	int frame_x;
	int frame_y;

	unsigned int swPlay;
	unsigned long color = GetCortBoxColor(RGB(0, 0, 0x20));

	swPlay = 1;
	
	//Load selected graphics (only when no mods are enabled, thank you Nicalis)
	SetOriginalGraphics(Filesystem_GetMod().empty() ? gConfig.original_graphics : FALSE);

	// Reset stuff
	gCounter = 0;
	grcGame.left = 0;
	grcGame.top = 0;
	grcGame.right = WINDOW_WIDTH;
	grcGame.bottom = WINDOW_HEIGHT;
	g_GameFlags = 3;

	// Initialize everything
	InitMyChar();
	InitNpChar();
	InitBullet();
	InitCaret();
	InitStar();
	InitFade();
	InitFlash();
	ClearArmsData();
	ClearItemData();
	ClearPermitStage();
	StartMapping();
	InitFlags();
	InitBossLife();

	if (!LoadProfile(gProfileId) && !InitializeGame(gProfileId))
		return 0;

	while (1)
	{
		// Get pressed keys
		GetTrg();

		if (swPlay % 2 && g_GameFlags & 1)	// The "swPlay % 2" part is always true
		{
			if (g_GameFlags & 2)
				ActMyChar(TRUE);
			else
				ActMyChar(FALSE);

			ActStar();
			ActNpChar();
			ActBossChar();
			ActValueView();
			ActBack();
			ResetMyCharFlag();
			HitMyCharMap();
			HitMyCharNpChar();
			HitMyCharBoss();
			HitNpCharMap();
			HitBossMap();
			HitBulletMap();
			HitNpCharBullet();
			HitBossBullet();
			if (g_GameFlags & 2)
				ShootBullet();
			ActBullet();
			ActCaret();
			MoveFrame3();
			GetFramePosition(&frame_x, &frame_y);
			ActFlash(frame_x, frame_y);

			if (g_GameFlags & 2)
				AnimationMyChar(TRUE);
			else
				AnimationMyChar(FALSE);
		}

		if (g_GameFlags & 8)
		{
			ActionCredit();
			ActionIllust();
			ActionStripper();
		}

		ProcFade();
		CortBox(&grcFull, color);
		GetFramePosition(&frame_x, &frame_y);
		PutBack(frame_x, frame_y);
		PutStage_Back(frame_x, frame_y);
		PutBossChar(frame_x, frame_y);
		PutNpChar(frame_x, frame_y);
		PutBullet(frame_x, frame_y);
		PutMyChar(frame_x, frame_y);
		PutStar(frame_x, frame_y);
		PutMapDataVector(frame_x, frame_y);
		PutStage_Front(frame_x, frame_y);
		PutFront(frame_x, frame_y);
		PutFlash();
		PutCaret(frame_x, frame_y);
		PutValueView(frame_x, frame_y);
		PutBossLife();
		PutFade();

		// Escape menu
		if (gKey & KEY_ESCAPE)
		{
			BackupSurface(SURFACE_ID_SCREEN_GRAB, &grcFull);
			
			switch (Call_Escape())
			{
				case enum_ESCRETURN_exit:
					return 0;
				case enum_ESCRETURN_restart:
					return 1;
				default:
					break;
			}
			
			PutBitmap4(&grcFull, 0, 0, &grcFull, SURFACE_ID_SCREEN_GRAB);
		}

		if (!(g_GameFlags & 4))
		{
			// Open inventory
			if (gKeyTrg & gKeyItem)
			{
				BackupSurface(SURFACE_ID_SCREEN_GRAB, &grcGame);

				switch (CampLoop())
				{
					case enum_ESCRETURN_exit:
						return 0;

					case enum_ESCRETURN_restart:
						return 1;
				}

				PutBitmap4(&grcGame, 0, 0, &grcGame, SURFACE_ID_SCREEN_GRAB);
				gMC.cond &= ~1;
			}
			else if (gMC.equip & EQUIP_MAP && gKeyTrg & gKeyMap)
			{
				BackupSurface(SURFACE_ID_SCREEN_GRAB, &grcGame);

				switch (MiniMapLoop())
				{
					case enum_ESCRETURN_exit:
						return 0;

					case enum_ESCRETURN_restart:
						return 1;
				}
				
				PutBitmap4(&grcGame, 0, 0, &grcGame, SURFACE_ID_SCREEN_GRAB);
			}
		}

		if (g_GameFlags & 2)
		{
			if (gKeyTrg & gKeyArms)
				RotationArms();
			else if (gKeyTrg & gKeyArmsRev)
				RotationArmsRev();
		}

		if (swPlay % 2)	// This is always true
		{
			switch (TextScriptProc())
			{
				case enum_ESCRETURN_exit:
					return 0;

				case enum_ESCRETURN_restart:
					return 1;
			}
		}

		PutMapName(FALSE);
		PutTimeCounter(16, 8);

		if (g_GameFlags & 2)
		{
			PutMyLife(TRUE);
			PutArmsEnergy(TRUE);
			PutMyAir((WINDOW_WIDTH / 2) - 40, (WINDOW_HEIGHT / 2) - 16);
			PutActiveArmsList();
		}

		if (g_GameFlags & 8)
		{
			PutIllust();
			PutStripper();
		}

		PutTextScript();

		PutFramePerSecound();

		if (!Flip_SystemTask())
			return 0;

		++gCounter;
	}

	return 0;
}

static int ModeNicalis()
{
	// Reset cliprect and flags
	grcGame.left = 0;
	grcGame.top = 0;
	grcGame.right = WINDOW_WIDTH;
	grcGame.bottom = WINDOW_HEIGHT;
	
	//Nicalis logo
	RECT rcNicalis = {0, 0, 256, 32};
	
	//Initialize fading and timing
	int timer = 0, mode = 0;
	InitFade();
	StartFadeIn(4);
	
	while (1)
	{
		//Draw background and Nicalis logo
		CortBox(&grcGame, 0);
		PutBitmap3(&grcGame, (WINDOW_WIDTH - rcNicalis.right) / 2, (WINDOW_HEIGHT - rcNicalis.bottom) / 2, &rcNicalis, SURFACE_ID_NICALIS);
		
		//Handle fading
		switch (mode)
		{
			case 0:
				PutFade();
				ProcFade();
				if (!GetFadeActive())
					mode = 1;
				break;
			case 1:
				if (++timer >= 60)
				{
					mode = 2;
					StartFadeOut(0);
				}
				break;
			case 2:
				PutFade();
				ProcFade();
				if (!GetFadeActive())
				{
					if (!TransitionWait())
						return 0;
					return 1;
				}
		}
		
		PutFramePerSecound();
		if (!Flip_SystemTask())
			return 0;
	}
}

BOOL Game(void)
{
	int mode;

	//Initialize and load stuff
	MakeGenericSurfaces();
	Filesystem_SetMod("");
	if (!(LoadAssets() && InitTextScript2()))
		return FALSE;
	
	InitCreditScript();
	InitSkipFlags();

	//Enter game loop
	mode = 4;
	while (mode)
	{
		if (mode == 1)
			mode = ModeOpening();
		if (mode == 2)
			mode = ModeTitle();
		if (mode == 3)
			mode = ModeAction();
		if (mode == 4)
			mode = ModeNicalis();
	}

	//Deinitialize and free stuff
	EndMapData();
	EndTextScript();
	ReleaseNpcTable();
	ReleaseStageTable();
	ReleaseMusicTable();
	ReleaseCreditScript();
	return TRUE;
}
