#include "Stage.h"

#include <string>

#include "WindowsWrapper.h"

#include "Main.h"
#include "File.h"
#include "Back.h"
#include "Boss.h"
#include "Bullet.h"
#include "Caret.h"
#include "Draw.h"
#include "Flash.h"
#include "Frame.h"
#include "Map.h"
#include "MapName.h"
#include "MyChar.h"
#include "NpChar.h"
#include "Music.h"
#include "TextScr.h"
#include "ValueView.h"
#include "Filesystem.h"

#ifdef JAPANESE
#define STAGE_ENTRY(parts, map, bkType, back, npc, boss, boss_no, name_en, name_jp) {parts, map, bkType, back, npc, boss, boss_no, name_jp}
#else
#define STAGE_ENTRY(parts, map, bkType, back, npc, boss, boss_no, name_en, name_jp) {parts, map, bkType, back, npc, boss, boss_no, name_en}
#endif

int gStageNo;
MusicID gMusicNo;
unsigned int gOldPos;
MusicID gOldNo;

STAGE_TABLE *gTMT = nullptr;
size_t gTMT_size;

BOOL LoadStageTable()
{
	FILE *fp = OpenFile(FSS_Mod, "stage.tbl", "rb");
	if (!fp)
		return FALSE;
	
	fseek(fp, 0, SEEK_END);
	gTMT_size = ftell(fp) / 0xE5;
	fseek(fp, 0, SEEK_SET);
	
	free(gTMT);
	if ((gTMT = (STAGE_TABLE*)malloc(gTMT_size * sizeof(STAGE_TABLE))) == NULL)
		return FALSE;
	
	for (size_t i = 0; i < gTMT_size; i++)
	{
		fread(gTMT[i].parts, 0x20, 1, fp);
		fread(gTMT[i].map, 0x20, 1, fp);
		gTMT[i].bkType = File_ReadLE32(fp);
		fread(gTMT[i].back, 0x20, 1, fp);
		fread(gTMT[i].npc, 0x20, 1, fp);
		fread(gTMT[i].boss, 0x20, 1, fp);
		gTMT[i].boss_no = fgetc(fp);
		fread(gTMT[i].name_jp, 0x20, 1, fp);
		fread(gTMT[i].name, 0x20, 1, fp);
	}
	
	fclose(fp);
	return TRUE;
}

void FreeStageTable()
{
	free(gTMT);
}

BOOL TransferStage(int no, int w, int x, int y)
{
	std::string path;
	std::string path_dir;
	BOOL bError;
	
	// If we can't load from the stage table, fail immediately
	if (gTMT == NULL || no > gTMT_size)
		return FALSE;
	
	// Move character
	SetMyCharPosition(x * 0x10 * 0x200, y * 0x10 * 0x200);

	bError = FALSE;

	// Get path
	path_dir = "Stage";

	// Load tileset
	path = path_dir + "/Prt" + gTMT[no].parts;
	if (!ReloadBitmap_File(path.c_str(), SURFACE_ID_LEVEL_TILESET))
		bError = TRUE;

	path = path_dir + '/' + gTMT[no].parts + ".pxa";
	if (!LoadAttributeData(path.c_str()))
		bError = TRUE;

	// Load tilemap
	path = path_dir + '/' + gTMT[no].map + ".pxm";
	if (!LoadMapData2(path.c_str()))
		bError = TRUE;

	// Load NPCs
	path = path_dir + '/' + gTMT[no].map + ".pxe";
	if (!LoadEvent(path.c_str()))
		bError = TRUE;

	// Load script
	path = path_dir + '/' + gTMT[no].map + ".tsc";
	if (!LoadTextScript_Stage(path.c_str()))
		bError = TRUE;

	// Load background
	path = gTMT[no].back;
	if (!InitBack(path.c_str(), gTMT[no].bkType))
		bError = TRUE;

	// Get path
	path_dir = "Npc";

	// Load NPC sprite sheets
	path = path_dir + "/Npc" + gTMT[no].npc;
	if (!ReloadBitmap_File(path.c_str(), SURFACE_ID_LEVEL_SPRITESET_1))
		bError = TRUE;

	path = path_dir + "/Npc" + gTMT[no].boss;
	if (!ReloadBitmap_File(path.c_str(), SURFACE_ID_LEVEL_SPRITESET_2))
		bError = TRUE;

	if (bError)
		return FALSE;

	// Load map name
	ReadyMapName(gTMT[no].name);

	StartTextScript(w);
	SetFrameMyChar();
	ClearBullet();
	InitCaret();
	ClearValueView();
	ResetQuake();
	InitBossChar(gTMT[no].boss_no);
	ResetFlash();
	gStageNo = no;

	return TRUE;
}

// Music
char **gMusicTable = nullptr;
size_t gMusicTable_size;

BOOL LoadMusicTable()
{
	FILE *fp = OpenFile(FSS_Mod, "music.tbl", "rb");
	if (!fp)
		return FALSE;
	
	fseek(fp, 0, SEEK_END);
	gMusicTable_size = ftell(fp) / 0x10;
	fseek(fp, 0, SEEK_SET);
	
	free(gMusicTable);
	if ((gMusicTable = (char**)malloc(gMusicTable_size * sizeof(char**))) == NULL)
		return FALSE;
	
	char **musicp = gMusicTable;
	for (size_t i = 0; i < gMusicTable_size; i++)
	{
		*musicp = (char*)malloc(0x10);
		fread(*musicp, 0x10, 1, fp);
		musicp++;
	}
	
	fclose(fp);
	return TRUE;
}

void FreeMusicTable()
{
	free(gMusicTable);
}

void ChangeMusic(MusicID no)
{
	if (no != MUS_SILENCE && no == gMusicNo)
		return;
	
	// Stop and keep track of old song
	gOldPos = GetMusicPosition();
	gOldNo = gMusicNo;
	StopMusic();

	// Load music
	if (gMusicTable != NULL && no < gMusicTable_size)
		LoadMusic(gMusicTable[no]);

	// Reset position, volume, and then play the song
	ChangeMusicVolume(100);
	SetMusicPosition(0);
	PlayMusic();
	gMusicNo = no;
}

void ReCallMusic(void)
{
	// Stop old music
	StopMusic();

	// Load music that was playing before
	LoadMusic(gMusicTable[gOldNo]);

	// Reset position, volume, and then play the song
	SetMusicPosition(gOldPos);
	ChangeMusicVolume(100);
	PlayMusic();
	gMusicNo = gOldNo;
}
