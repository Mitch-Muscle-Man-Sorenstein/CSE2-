#include "Ending.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "WindowsWrapper.h"

#include "Draw.h"
#include "Escape.h"
#include "Flags.h"
#include "Generic.h"
#include "KeyControl.h"
#include "Main.h"
#include "MycParam.h"
#include "Music.h"
#include "Stage.h"
#include "TextScr.h"
#include "File.h"
#include "Filesystem.h"
#include "MyChar.h"

struct CREDIT
{
	long size;
	char *pData;
	int offset;
	int wait;
	int mode;
	int start_x;
};

struct STRIP
{
	int flag;
	int x;
	int y;
	int cast;
	char str[0x40];
};

struct ILLUSTRATION
{
	int act_no;
	int x;
};

struct ISLAND_SPRITE
{
	int x;
	int y;
};

static CREDIT Credit;
static STRIP Strip[MAX_STRIP];
static ILLUSTRATION Illust;

// Update casts
void ActionStripper(void)
{
	int s;

	for (s = 0; s < MAX_STRIP; ++s)
	{
		// Move up
		if (Strip[s].flag & 0x80 && Credit.mode)
			Strip[s].y -= 0x100;
		// Get removed when off-screen
		if (Strip[s].y <= -0x2000)
			Strip[s].flag = 0;
	}
}

// Draw casts
void PutStripper(void)
{
	int s;
	RECT rc;

	for (s = 0; s < MAX_STRIP; ++s)
	{
		if (Strip[s].flag & 0x80)
		{
			// Draw text
			PutText((Strip[s].x / 0x200) + ((WINDOW_WIDTH - 320) / 2), (Strip[s].y / 0x200), Strip[s].str, 0xFFFFFF);

			// Draw character
			rc.left = (Strip[s].cast % 13) * 24;
			rc.right = rc.left + 24;
			rc.top = (Strip[s].cast / 13) * 24;
			rc.bottom = rc.top + 24;

			PutBitmap3(&grcFull, (Strip[s].x / 0x200) + ((WINDOW_WIDTH - 320) / 2) - 24, (Strip[s].y / 0x200) - 8, &rc, SURFACE_ID_CASTS);
		}
	}
}

// Create a cast object
void SetStripper(int x, int y, const char *text, int cast)
{
	int s;
	RECT rc;

	for (s = 0; s < MAX_STRIP; ++s)
		if (!(Strip[s].flag & 0x80))
			break;

	if (s == MAX_STRIP)
		return;

	// Initialize cast property
	Strip[s].flag = 0x80;
	Strip[s].x = x;
	Strip[s].y = y;
	Strip[s].cast = cast;
	strcpy(Strip[s].str, text);

	// Draw text
	rc.left = 0;
	rc.right = 320;
	rc.top = s * 16;
	rc.bottom = rc.top + 16;
}

// Regenerate cast text
void RestoreStripper(void)
{
	return;
}

// Handle the illustration
void ActionIllust(void)
{
	switch (Illust.act_no)
	{
		case 0: // Off-screen to the left
			Illust.x = -160 * 0x200;
			break;

		case 1: // Move in from the left
			Illust.x += 40 * 0x200;
			if (Illust.x > 0)
				Illust.x = 0;
			break;

		case 2: // Move out from the right
			Illust.x -= 0x5000;
			if (Illust.x < -160 * 0x200)
				Illust.x = -160 * 0x200;
			break;
	}
}

// Draw illustration
void PutIllust(void)
{
	RECT rcIllust = {0, 0, 160, 240};
	RECT rcClip = {(WINDOW_WIDTH - 320) / 2, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
	PutBitmap3(&rcClip, (Illust.x / 0x200) + ((WINDOW_WIDTH - 320) / 2), (WINDOW_HEIGHT - 240) / 2, &rcIllust, SURFACE_ID_CREDITS_IMAGE);
}

// Load illustration
void ReloadIllust(int a)
{
	char name[16];
	if (gMC.equip & EQUIP_MIMIGA_MASK)
	{
		sprintf(name, "CREDIT%02d%c", a, gUseOriginalGraphics ? 'b' : 'a');
		if (ReloadBitmap_Resource(name, SURFACE_ID_CREDITS_IMAGE))
			return;
	}
	sprintf(name, "CREDIT%02d", a);
	ReloadBitmap_Resource(name, SURFACE_ID_CREDITS_IMAGE);
}

std::string credit_script = "credit.tsc";

// Initialize and release credits
void InitCreditScript(void)
{
	// Clear script state and casts
	memset(&Credit, 0, sizeof(CREDIT));
	memset(Strip, 0, sizeof(Strip));
}

void ReleaseCreditScript(void)
{
	if (Credit.pData != NULL)
	{
		//Free script data
		free(Credit.pData);
		Credit.pData = NULL;
	}
}

// Start playing credits
BOOL StartCreditScript(void)
{
	//Read credit script
	free(Credit.pData);
	if ((Credit.pData = ReadTextScript(credit_script, &Credit.size)) == nullptr)
		return FALSE;
	
	//Reset credits
	Credit.offset = 0;
	Credit.wait = 0;
	Credit.mode = 1;
	Illust.x = -160 * 0x200;
	Illust.act_no = 0;
	
	//Modify cliprect
	grcGame.left = WINDOW_WIDTH / 2;
	grcGame.right = ((WINDOW_WIDTH - 320) / 2) + 320;
	grcGame.top = (WINDOW_HEIGHT - 240) / 2;
	grcGame.bottom = ((WINDOW_HEIGHT - 240) / 2) + 240;
	
	//Reload casts
	if (!ReloadBitmap_File("casts", SURFACE_ID_CASTS))
		return FALSE;
	
	//Clear casts
	memset(Strip, 0, sizeof(Strip));
	return TRUE;
}

// Get number from text (4 digit)
static int GetScriptNumber(const char *text)
{
	return (text[0] - '0') * 1000 +
		(text[1] - '0') * 100 +
		(text[2] - '0') * 10 +
		text[3] - '0';
}

// Parse credits
static void ActionCredit_Read(void)
{
	int a, b, len;
	char text[40];

	while (1)
	{
		if (Credit.offset >= Credit.size)
			break;

		switch (Credit.pData[Credit.offset])
		{
			case '[': // Create cast
				// Get the range for the cast text
				++Credit.offset;

				a = Credit.offset;

				while (Credit.pData[a] != ']')
				{
					if (IsShiftJIS(Credit.pData[a]))
						a += 2;
					else
						a += 1;
				}

				len = a - Credit.offset;

				// Copy the text to the cast text
				memcpy(text, &Credit.pData[Credit.offset], len);
				text[len] = 0;

				// Get cast id
				Credit.offset = a;
				len = GetScriptNumber(&Credit.pData[++Credit.offset]);

				// Create cast object
				SetStripper(Credit.start_x, (WINDOW_HEIGHT * 0x200) + (8 * 0x200), text, len);

				// Change offset
				Credit.offset += 4;
				return;

			case '-': // Wait for X amount of frames
				++Credit.offset;
				Credit.wait = GetScriptNumber(&Credit.pData[Credit.offset]);
				Credit.offset += 4;
				Credit.mode = 2;
				return;

			case '+': // Change casts x-position
				++Credit.offset;
				Credit.start_x = GetScriptNumber(&Credit.pData[Credit.offset]) * 0x200;
				Credit.offset += 4;
				return;

			case '/': // Stop credits
				Credit.mode = 0;
				return;

			case '!': // Change music
				++Credit.offset;
				a = GetScriptNumber(&Credit.pData[Credit.offset]);
				Credit.offset += 4;
				ChangeMusic((MusicID)a);
				return;

			case '~': // Start fading out music
				++Credit.offset;
				SetMusicFadeout();
				return;

			case 'j': // Jump to label
				++Credit.offset;

				// Get number
				b = GetScriptNumber(&Credit.pData[Credit.offset]);

				// Change offset
				Credit.offset += 4;

				// Jump to specific label
				if (1)
				{
					while (Credit.offset < Credit.size)
					{
						if (Credit.pData[Credit.offset] == 'l')
						{
							// What is this
							a = GetScriptNumber(&Credit.pData[++Credit.offset]);
							Credit.offset += 4;
							if (b == a)
								break;
						}
						else if (IsShiftJIS(Credit.pData[Credit.offset]))
						{
							Credit.offset += 2;
						}
						else
						{
							++Credit.offset;
						}
					}
				}

				return;

			case 'f': // Flag jump
				++Credit.offset;

				// Read numbers XXXX:YYYY
				a = GetScriptNumber(&Credit.pData[Credit.offset]);
				Credit.offset += 5;
				b = GetScriptNumber(&Credit.pData[Credit.offset]);
				Credit.offset += 4;

				// If flag is set
				if (GetNPCFlag(a))
				{
					// Jump to label
					while (Credit.offset < Credit.size)
					{
						if (Credit.pData[Credit.offset] == 'l')
						{
							a = GetScriptNumber(&Credit.pData[++Credit.offset]);
							Credit.offset += 4;
							if (b == a)
								break;
						}
						else if (IsShiftJIS(Credit.pData[Credit.offset]))
						{
							Credit.offset += 2;
						}
						else
						{
							++Credit.offset;
						}
					}
				}
				return;

			default:
				// Progress through file
				++Credit.offset;
				break;
		}
	}
}

// Update credits
void ActionCredit(void)
{
	if (Credit.offset >= Credit.size)
		return;

	// Update script, or if waiting, decrement the wait value
	switch (Credit.mode)
	{
		case 1:
			ActionCredit_Read();
			break;

		case 2:
			if (--Credit.wait <= 0)
				Credit.mode = 1;
			break;
	}
}

// Change illustration
void SetCreditIllust(int a)
{
	ReloadIllust(a);
	Illust.act_no = 1;
}

// Slide illustration off-screen
void CutCreditIllust(void)
{
	Illust.act_no = 2;
}

// Scene of the island falling
int Scene_DownIsland(int mode)
{
	ISLAND_SPRITE sprite;
	int wait;

	// Setup background
	RECT rc_frame = {(WINDOW_WIDTH / 2) - 80, (WINDOW_HEIGHT / 2) - 40, (WINDOW_WIDTH / 2) + 80, (WINDOW_HEIGHT / 2) + 40};
	RECT rc_sky = {0, 0, 160, 80};
	RECT rc_ground = {160, 48, 320, 80};

	// Setup island
	RECT rc_sprite = {160, 0, 200, 24};

	sprite.x = 168 * 0x200;
	sprite.y = 64 * 0x200;

	for (wait = 0; wait < 900; ++wait)
	{
		// Get pressed keys
		GetTrg();

		// Escape menu
		if (gKeyTrg & KEY_ESCAPE)
		{
			BackupSurface(SURFACE_ID_SCREEN_GRAB, &grcGame);
			
			switch (Call_Escape())
			{
				case enum_ESCRETURN_exit:
					return enum_ESCRETURN_exit;
				case enum_ESCRETURN_restart:
					return enum_ESCRETURN_restart;
				default:
					break;
			}
			
			PutBitmap4(&grcGame, 0, 0, &grcGame, SURFACE_ID_SCREEN_GRAB);
		}

		switch (mode)
		{
			case 0:
				// Move down
				sprite.y += 0x33;
				break;

			case 1:
				if (wait < 350)
				{
					// Move down at normal speed
					sprite.y += 0x33;
				}
				else if (wait < 500)
				{
					// Move down slower
					sprite.y += 0x19;
				}
				else if (wait < 600)
				{
					// Move down slow
					sprite.y += 0xC;
				}
				else if (wait == 750)
				{
					// End scene
					wait = 900;
				}

				break;
		}

		// Draw scene
		CortBox(&grcFull, 0);
		PutBitmap3(&rc_frame, 80 + ((WINDOW_WIDTH - 320) / 2), 80 + ((WINDOW_HEIGHT - 240) / 2), &rc_sky, SURFACE_ID_LEVEL_SPRITESET_1);
		PutBitmap3(&rc_frame, (sprite.x / 0x200) - 20 + ((WINDOW_WIDTH - 320) / 2), (sprite.y / 0x200) - 12 + ((WINDOW_HEIGHT - 240) / 2), &rc_sprite, SURFACE_ID_LEVEL_SPRITESET_1);
		PutBitmap3(&rc_frame, 80 + ((WINDOW_WIDTH - 320) / 2), 128 + ((WINDOW_HEIGHT - 240) / 2), &rc_ground, SURFACE_ID_LEVEL_SPRITESET_1);
		PutTimeCounter(16, 8);

		// Draw window
		PutFramePerSecound();
		if (!Flip_SystemTask())
			return enum_ESCRETURN_exit;
	}

	return enum_ESCRETURN_continue;
}
