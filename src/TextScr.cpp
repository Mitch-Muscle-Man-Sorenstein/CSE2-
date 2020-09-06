#include "TextScr.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "WindowsWrapper.h"

#include "Backends/Misc.h"
#include "ArmsItem.h"
#include "Boss.h"
#include "BossLife.h"
#include "CommonDefines.h"
#include "Draw.h"
#include "Ending.h"
#include "Escape.h"
#include "Fade.h"
#include "Flags.h"
#include "Flash.h"
#include "Frame.h"
#include "Font.h"
#include "Game.h"
#include "Generic.h"
#include "KeyControl.h"
#include "Main.h"
#include "Map.h"
#include "MapName.h"
#include "MiniMap.h"
#include "MyChar.h"
#include "MycParam.h"
#include "NpChar.h"
#include "Music.h"
#include "Profile.h"
#include "SelStage.h"
#include "Sound.h"
#include "Stage.h"
#include "File.h"
#include "Filesystem.h"

//TSC constants and macros
#define TEXT_LEFT (WINDOW_WIDTH / 2 - 108)
#define IS_COMMAND(c1, c2, c3) (gTS.p_read[1] == (c1) && gTS.p_read[2] == (c2) && gTS.p_read[3] == (c3))

//TSC state
TEXT_SCRIPT gTS{};
static char text[4][0x40];

const RECT gRect_line = {0, 0, 216, 16};
static unsigned long nod_color;

//TSC decryption
void EncryptionBinaryData2(unsigned char *pData, long size)
{
	int i;
	int work;

	int half;
	int val1;

	half = size / 2;

	if (pData[half] == 0)
		val1 = -7;
	else
		val1 = (pData[half] % 0x100) * -1;

	for (i = 0; i < size; ++i)
	{
		work = pData[i];
		work += val1;

		if (i != half)
			pData[i] = work % 0x100;
	}
}

//Common TSC read function
char *ReadTextScript(std::string name, long *size)
{
	//Open file and get size
	FILE *fp = OpenFile(FSS_Mod, name, "rb");
	if (fp == NULL)
		return NULL;
	
	fseek(fp, 0, SEEK_END);
	long fp_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	//Allocate, read, and decrypt into a buffer
	char *data = new char[fp_size + 1];
	if (data != NULL)
	{
		fread(data, fp_size, 1, fp);
		EncryptionBinaryData2((unsigned char*)data, fp_size);
	}
	fclose(fp);
	
	//Append terminator key
	data[fp_size] = '\0';
	
	//Return size and data
	if (size != NULL)
		*size = fp_size;
	return data;
}

//TSC initialization and end
BOOL InitTextScript2(void)
{
	//Get NOD colour
	nod_color = GetCortBoxColor(RGB(0xFF, 0xFF, 0xFE));
	
	//Reset state
	gTS.mode = 0;
	
	//Clear text
	memset(text, 0, sizeof(text));
	
	//Read head script
	delete[] gTS.head;
	if ((gTS.head = ReadTextScript("Head.tsc", NULL)) == NULL)
		return FALSE;
	return TRUE;
}

void EndTextScript(void)
{
	//Free buffers
	delete[] gTS.head;
	delete[] gTS.data;
}

//Load generic .tsc
BOOL LoadTextScript2(std::string name)
{
	//Read given script
	delete[] gTS.data;
	if ((gTS.data = ReadTextScript(name, NULL)) == NULL)
		return FALSE;
	
	//Initialize other state things
	gTS.use_head = FALSE;
	gTS.path = name;
	return TRUE;
}

//Load stage .tsc
BOOL LoadTextScript_Stage(std::string name)
{
	//Read given script
	delete[] gTS.data;
	if ((gTS.data = ReadTextScript(name, NULL)) == NULL)
		return FALSE;
	
	//Initialize other state things
	gTS.use_head = TRUE;
	gTS.path = name;
	return TRUE;
}

//Get current path
std::string GetTextScriptPath(void)
{
	return gTS.path;
}

//Get 4 digit number from TSC data
int GetTextScriptNo(int a)
{
	int b = 0;
	b += (gTS.p_read[a++] - '0') * 1000;
	b += (gTS.p_read[a++] - '0') * 100;
	b += (gTS.p_read[a++] - '0') * 10;
	b += gTS.p_read[a] - '0';
	return b;
}

//TSC event seek
BOOL SeekTextScript_Single(int no, char *data)
{
	gTS.p_read = data;
	while (1)
	{
		//Check if we are still in the proper range
		if (*gTS.p_read == '\0')
			return FALSE;
		
		//Check if we are at an event
		if (*gTS.p_read == '#')
		{
			//Check if this is our event
			int event_no = GetTextScriptNo(1);
			if (no == event_no)
				break;
			if (no < event_no)
				return FALSE;
		}
		
		gTS.p_read++;
	}
	return TRUE;
}

BOOL SeekTextScript(int no)
{
	//Seek to event
	if (!((gTS.use_head && SeekTextScript_Single(no, gTS.head)) || SeekTextScript_Single(no, gTS.data)))
		return FALSE;
	
	//Advance until new-line
	while (*gTS.p_read != '\n')
		gTS.p_read++;
	gTS.p_read++;
	return TRUE;
}

//Start TSC event
BOOL StartTextScript(int no)
{
	//Reset state
	gTS.mode = 0;
	g_GameFlags |= 5;
	gTS.line = 0;
	gTS.wait = 4;
	gTS.flags = 0;
	gTS.wait_beam = 0;
	gTS.face = 0;
	gTS.item = 0;
	gTS.offsetY = 0;
	
	gTS.rcText.left = TEXT_LEFT;
	gTS.rcText.top = WINDOW_HEIGHT - 56;
	gTS.rcText.right = WINDOW_WIDTH - TEXT_LEFT;
	gTS.rcText.bottom = gTS.rcText.top + 48;
	
	//Seek to event
	if (!SeekTextScript(no))
		return FALSE;
	
	//Start event
	gTS.mode = 1;
	return TRUE;
}

BOOL JumpTextScript(int no)
{
	//Reset state
	gTS.mode = 0;
	g_GameFlags |= 4;
	gTS.line = 0;
	gTS.wait = 4;
	gTS.wait_beam = 0;
	
	//Clear text lines
	for (int i = 0; i < 4; ++i)
	{
		gTS.ypos_line[i] = i * 16;
		memset(text[i], 0, sizeof(text[0]));
	}
	
	//Seek to event
	if (!SeekTextScript(no))
		return FALSE;
	
	//Start event
	gTS.mode = 1;
	return TRUE;
}

//End event
void StopTextScript(void)
{
	// End TSC and reset flags
	gTS.mode = 0;
	g_GameFlags &= ~4;
	g_GameFlags |= 3;
	gTS.flags = 0;
}

//Prepare a new line
void CheckNewLine(void)
{
	if (gTS.ypos_line[gTS.line % 4] == 48)
	{
		gTS.mode = 3;
		g_GameFlags |= 4;
		memset(text[gTS.line % 4], 0, sizeof(text[0]));
	}
}

int gNumberTextScript[4];

//Type a number into the text buffer
void SetNumberTextScript(int index)
{
	//Digit table
	static const int table[3] = {
		1000,
		100,
		10,
	};
	
	//Get number to print
	int a = gNumberTextScript[index];
	
	BOOL bZero = FALSE;
	int offset = 0;
	
	//Write to string buffer
	char str[5];
	for (int i = 0; i < 3; ++i)
	{
		if (a / table[i] || bZero)
		{
			int b = a / table[i];
			str[offset] = '0' + (char)b;
			bZero = TRUE;
			a -= b * table[i];
			++offset;
		}
	}
	
	//Set last digit of string, and add null terminator
	str[offset] = '0' + (char)a;
	str[offset + 1] = '\0';

	//Append number to line
	strcat(text[gTS.line % 4], str);

	//Play sound and reset blinking cursor
	PlaySoundObject(2, 1);
	gTS.wait_beam = 0;
}

//Clear text lines
void ClearTextLine(void)
{
	gTS.line = 0;
	gTS.offsetY = 0;
	
	for (int i = 0; i < 4; ++i)
	{
		gTS.ypos_line[i] = i * 16;
		memset(text[i], 0, sizeof(text[0]));
	}
}

//Draw textbox and whatever else
void PutTextScript(void)
{
	int i;
	RECT rect;
	int text_offset;

	if (gTS.mode == 0)
		return;

	if ((gTS.flags & 1) == 0)
		return;

	// Set textbox position
	if (gTS.flags & 0x20)
	{
		gTS.rcText.top = 32;
		gTS.rcText.bottom = gTS.rcText.top + 48;
	}
	else
	{
		gTS.rcText.top = WINDOW_HEIGHT - 72;
		gTS.rcText.bottom = gTS.rcText.top + 48;
	}

	// Draw textbox
	if (gTS.flags & 2)
	{
		RECT rcFrame1 = {0, 0, 244, 8};
		RECT rcFrame2 = {0, 8, 244, 16};
		RECT rcFrame3 = {0, 16, 244, 24};

		PutBitmap3(&grcFull, WINDOW_WIDTH / 2 - 122, gTS.rcText.top - 10, &rcFrame1, SURFACE_ID_TEXT_BOX);
		for (i = 1; i < 7; ++i)
			PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) - 122, (i * 8) + gTS.rcText.top - 10, &rcFrame2, SURFACE_ID_TEXT_BOX);
		PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) - 122, (i * 8) + gTS.rcText.top - 10, &rcFrame3, SURFACE_ID_TEXT_BOX);
	}

	// Draw face picture
	RECT rcFace;
	rcFace.left = (gTS.face % 6) * 48;
	rcFace.top = (gTS.face / 6) * 48;
	rcFace.right = rcFace.left + 48;
	rcFace.bottom = rcFace.top + 48;

	if (gTS.face_x < (TEXT_LEFT * 0x200))
		gTS.face_x += 0x1000;

	PutBitmap3(&gTS.rcText, SubpixelToScreen(gTS.face_x), gTS.rcText.top - 3, &rcFace, SURFACE_ID_FACE);

	// Draw text
	if (gTS.face != 0)
		text_offset = 68;
	else
		text_offset = 12;

	for (i = 0; i < 4; ++i)
	{
		if (text[i][0] && gTS.ypos_line[i] != 48 && (gTS.ypos_line[i] > 0 || gTS.mode != 3))
			PutText(TEXT_LEFT - 12 + text_offset, gTS.offsetY + gTS.ypos_line[i] + gTS.rcText.top + 1, text[i], RGB(0xFF, 0xFF, 0xFE));
	}

	// Draw NOD cursor
	if ((gTS.wait_beam++ % 20 > 12) && gTS.mode == 2)
	{
		rect.left = TEXT_LEFT - 12 + GetTextWidth(text[gTS.line % 4]) + text_offset + 1;
		rect.top = gTS.ypos_line[gTS.line % 4] + gTS.rcText.top + gTS.offsetY;
		rect.right = rect.left + 7;
		rect.bottom = rect.top + 11;
		CortBox(&rect, nod_color);
	}

	// Draw GIT
	RECT rcItemBox1 = {0, 0, 72, 16};
	RECT rcItemBox2 = {0, 8, 72, 24};
	RECT rcItemBox3 = {240, 0, 244, 8};
	RECT rcItemBox4 = {240, 8, 244, 16};
	RECT rcItemBox5 = {240, 16, 244, 24};

	if (gTS.item != 0)
	{
		PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) - 40, WINDOW_HEIGHT - 112, &rcItemBox1, SURFACE_ID_TEXT_BOX);
		PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) - 40, WINDOW_HEIGHT - 96, &rcItemBox2, SURFACE_ID_TEXT_BOX);
		PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) + 32, WINDOW_HEIGHT - 112, &rcItemBox3, SURFACE_ID_TEXT_BOX);
		PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) + 32, WINDOW_HEIGHT - 104, &rcItemBox4, SURFACE_ID_TEXT_BOX);
		PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) + 32, WINDOW_HEIGHT - 96, &rcItemBox4, SURFACE_ID_TEXT_BOX);
		PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) + 32, WINDOW_HEIGHT - 88, &rcItemBox5, SURFACE_ID_TEXT_BOX);

		if (gTS.item_y < WINDOW_HEIGHT - 104)
			++gTS.item_y;

		if (gTS.item < 1000)
		{
			rect.left = (gTS.item % 16) * 16;
			rect.right = rect.left + 16;
			rect.top = (gTS.item / 16) * 16;
			rect.bottom = rect.top + 16;
			PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) - 9, gTS.item_y, &rect, SURFACE_ID_ARMS_IMAGE);
		}
		else
		{
			rect.left = 32 * ((gTS.item - 1000) % 8);
			rect.right = rect.left + 32;
			rect.top = 16 * ((gTS.item - 1000) / 8);
			rect.bottom = rect.top + 16;
			PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) - 17, gTS.item_y, &rect, SURFACE_ID_ITEM_IMAGE);
		}
	}

	// Draw Yes / No selection
	RECT rect_yesno = {152, 48, 244, 80};
	RECT rect_cur = {112, 88, 128, 104};

	if (gTS.mode == 6)
	{
		if (gTS.wait < 2)
			i = (WINDOW_HEIGHT - 96) + (2 - gTS.wait) * 4;
		else
			i = WINDOW_HEIGHT - 96;

		PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) + 74, i, &rect_yesno, SURFACE_ID_TEXT_BOX);
		if (gTS.wait == 16)
			PutBitmap3(&grcFull, (gTS.select * 41) + (WINDOW_WIDTH / 2) + 69, WINDOW_HEIGHT - 86, &rect_cur, SURFACE_ID_TEXT_BOX);
	}
}

// Parse TSC
int TextScriptProc(void)
{
	int i;
	char c[3];
	char str[72];
	int w, x, y, z;

	BOOL bExit;

	RECT rcSymbol = {64, 48, 72, 56};

	switch (gTS.mode)
	{
		case 1: // PARSE
			// Type out (faster if ok or cancel are held)
			++gTS.wait;

			if (!(g_GameFlags & 2) && gKey & (gKeyOk | gKeyCancel))
				gTS.wait += 4;

			if (gTS.wait < 4)
				break;

			gTS.wait = 0;

			// Parsing time
			bExit = FALSE;

			while (!bExit)
			{
				if (*gTS.p_read == '<')
				{
					if (IS_COMMAND('E','N','D'))
					{
						gTS.mode = 0;
						gMC.cond &= ~1;
						g_GameFlags |= 3;
						gTS.face = 0;
						bExit = TRUE;
					}
					else if (IS_COMMAND('L','I','+'))
					{
						x = GetTextScriptNo(4);
						AddLifeMyChar(x);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('M','L','+'))
					{
						z = GetTextScriptNo(4);
						AddMaxLifeMyChar(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('A','E','+'))
					{
						FullArmsEnergy();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('I','T','+'))
					{
						x = GetTextScriptNo(4);
						PlaySoundObject(38, 1);
						AddItemData(x);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('I','T','-'))
					{
						z = GetTextScriptNo(4);
						SubItemData(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('E','Q','+'))
					{
						z = GetTextScriptNo(4);
						EquipItem(z, TRUE);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('E','Q','-'))
					{
						z = GetTextScriptNo(4);
						EquipItem(z, FALSE);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('A','M','+'))
					{
						w = GetTextScriptNo(4);
						x = GetTextScriptNo(9);

						gNumberTextScript[0] = x;

						PlaySoundObject(38, 1);
						AddArmsData(w, x);
						gTS.p_read += 13;
					}
					else if (IS_COMMAND('A','M','-'))
					{
						z = GetTextScriptNo(4);
						SubArmsData(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('Z','A','M'))
					{
						ZeroArmsEnergy_All();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('T','A','M'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						z = GetTextScriptNo(14);
						TradeArms(x, y, z);
						gTS.p_read += 18;
					}
					else if (IS_COMMAND('P','S','+'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						AddPermitStage(x, y);
						gTS.p_read += 13;
					}
					else if (IS_COMMAND('M','P','+'))
					{
						x = GetTextScriptNo(4);
						SetMapping(x);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('U','N','I'))
					{
						z = GetTextScriptNo(4);
						ChangeMyUnit(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('S','T','C'))
					{
						SaveTimeCounter();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('T','R','A'))
					{
						z = GetTextScriptNo(4);
						w = GetTextScriptNo(9);
						x = GetTextScriptNo(14);
						y = GetTextScriptNo(19);

						if (!TransferStage(z, w, x, y))
						{
							#ifdef JAPANESE
							Backend_ShowMessageBox("エラー", "ステージの読み込みに失敗");
							#else
							Backend_ShowMessageBox("Error", "Failed to load stage");
							#endif

							return enum_ESCRETURN_exit;
						}
					}
					else if (IS_COMMAND('M','O','V'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						SetMyCharPosition(x * 0x200 * 0x10, y * 0x200 * 0x10);
						gTS.p_read += 13;
					}
					else if (IS_COMMAND('H','M','C'))
					{
						ShowMyChar(FALSE);
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('S','M','C'))
					{
						ShowMyChar(TRUE);
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('F','L','+'))
					{
						z = GetTextScriptNo(4);
						SetNPCFlag(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('F','L','-'))
					{
						z = GetTextScriptNo(4);
						CutNPCFlag(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('S','K','+'))
					{
						z = GetTextScriptNo(4);
						SetSkipFlag(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('S','K','-'))
					{
						z = GetTextScriptNo(4);
						CutSkipFlag(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('K','E','Y'))
					{
						g_GameFlags &= ~2;
						g_GameFlags |= 1;
						gMC.up = FALSE;
						gMC.shock = 0;
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('P','R','I'))
					{
						g_GameFlags &= ~3;
						gMC.shock = 0;
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('F','R','E'))
					{
						g_GameFlags |= 3;
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('N','O','D'))
					{
						gTS.mode = 2;
						gTS.p_read += 4;
						bExit = TRUE;
					}
					else if (IS_COMMAND('C','L','R'))
					{
						ClearTextLine();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('M','S','G'))
					{
						ClearTextLine();
						gTS.flags |= 0x03;
						gTS.flags &= ~0x30;
						if (gTS.flags & 0x40)
							gTS.flags |= 0x10;
						gTS.p_read += 4;
						bExit = TRUE;
					}
					else if (IS_COMMAND('M','S','2'))
					{
						ClearTextLine();
						gTS.flags &= ~0x12;
						gTS.flags |= 0x21;
						if (gTS.flags & 0x40)
							gTS.flags |= 0x10;
						gTS.face = 0;
						gTS.p_read += 4;
						bExit = TRUE;
					}
					else if (IS_COMMAND('M','S','3'))
					{
						ClearTextLine();
						gTS.flags &= ~0x10;
						gTS.flags |= 0x23;
						if (gTS.flags & 0x40)
							gTS.flags |= 0x10;
						gTS.p_read += 4;
						bExit = TRUE;
					}
					else if (IS_COMMAND('W','A','I'))
					{
						gTS.mode = 4;
						gTS.wait_next = GetTextScriptNo(4);
						gTS.p_read += 8;
						bExit = TRUE;
					}
					else if (IS_COMMAND('W','A','S'))
					{
						gTS.mode = 7;
						gTS.p_read += 4;
						bExit = TRUE;
					}
					else if (IS_COMMAND('T','U','R'))
					{
						gTS.p_read += 4;
						gTS.flags |= 0x10;
					}
					else if (IS_COMMAND('S','A','T'))
					{
						gTS.p_read += 4;
						gTS.flags |= 0x40;
					}
					else if (IS_COMMAND('C','A','T'))
					{
						gTS.p_read += 4;
						gTS.flags |= 0x40;
					}
					else if (IS_COMMAND('C','L','O'))
					{
						gTS.flags &= ~0x33;
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('E','V','E'))
					{
						z = GetTextScriptNo(4);
						JumpTextScript(z);
					}
					else if (IS_COMMAND('Y','N','J'))
					{
						gTS.next_event = GetTextScriptNo(4);
						gTS.p_read += 8;
						gTS.mode = 6;
						PlaySoundObject(5, 1);
						gTS.wait = 0;
						gTS.select = 0;
						bExit = TRUE;
					}
					else if (IS_COMMAND('F','L','J'))
					{
						x = GetTextScriptNo(4);
						z = GetTextScriptNo(9);

						if (GetNPCFlag(x))
							JumpTextScript(z);
						else
							gTS.p_read += 13;
					}
					else if (IS_COMMAND('S','K','J'))
					{
						x = GetTextScriptNo(4);
						z = GetTextScriptNo(9);

						if (GetSkipFlag(x))
							JumpTextScript(z);
						else
							gTS.p_read += 13;
					}
					else if (IS_COMMAND('I','T','J'))
					{
						x = GetTextScriptNo(4);
						z = GetTextScriptNo(9);

						if (CheckItem(x))
							JumpTextScript(z);
						else
							gTS.p_read += 13;
					}
					else if (IS_COMMAND('A','M','J'))
					{
						x = GetTextScriptNo(4);
						z = GetTextScriptNo(9);

						if (CheckArms(x))
							JumpTextScript(z);
						else
							gTS.p_read += 13;
					}
					else if (IS_COMMAND('U','N','J'))
					{
						x = GetTextScriptNo(4);
						z = GetTextScriptNo(9);
						if (GetUnitMyChar() == x)
							JumpTextScript(z);
						else
							gTS.p_read += 13;
					}
					else if (IS_COMMAND('E','C','J'))
					{
						x = GetTextScriptNo(4);
						z = GetTextScriptNo(9);
						if (GetNpCharAlive(x))
							JumpTextScript(z);
						else
							gTS.p_read += 13;
					}
					else if (IS_COMMAND('N','C','J'))
					{
						x = GetTextScriptNo(4);
						z = GetTextScriptNo(9);
						if (IsNpCharCode(x))
							JumpTextScript(z);
						else
							gTS.p_read += 13;
					}
					else if (IS_COMMAND('M','P','J'))
					{
						x = GetTextScriptNo(4);
						if (IsMapping())
							JumpTextScript(x);
						else
							gTS.p_read += 8;
					}
					else if (IS_COMMAND('S','S','S'))
					{
						x = GetTextScriptNo(4);
						SetNoise(1, x);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('C','S','S'))
					{
						CutNoise();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('S','P','S'))
					{
						SetNoise(2, 0);
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('C','P','S'))
					{
						CutNoise();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('Q','U','A'))
					{
						z = GetTextScriptNo(4);
						SetQuake(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('F','L','A'))
					{
						SetFlash(0, 0, 2);
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('F','A','I'))
					{
						z = GetTextScriptNo(4);
						StartFadeIn(z);
						gTS.mode = 5;
						gTS.p_read += 8;
						bExit = TRUE;
					}
					else if (IS_COMMAND('F','A','O'))
					{
						z = GetTextScriptNo(4);
						StartFadeOut(z);
						gTS.mode = 5;
						gTS.p_read += 8;
						bExit = TRUE;
					}
					else if (IS_COMMAND('M','N','A'))
					{
						StartMapName();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('F','O','M'))
					{
						z = GetTextScriptNo(4);
						SetFrameTargetMyChar(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('F','O','N'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						SetFrameTargetNpChar(x, y);
						gTS.p_read += 13;
					}
					else if (IS_COMMAND('F','O','B'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						SetFrameTargetBoss(x, y);
						gTS.p_read += 13;
					}
					else if (IS_COMMAND('S','O','U'))
					{
						z = GetTextScriptNo(4);
						PlaySoundObject(z, 1);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('C','M','U'))
					{
						z = GetTextScriptNo(4);
						ChangeMusic((MusicID)z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('F','M','U'))
					{
						SetMusicFadeout();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('R','M','U'))
					{
						ReCallMusic();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('M','L','P'))
					{
						gTS.p_read += 4;
						bExit = TRUE;

						switch (MiniMapLoop())
						{
							case enum_ESCRETURN_exit:
								return enum_ESCRETURN_exit;

							case enum_ESCRETURN_restart:
								return enum_ESCRETURN_restart;
						}
					}
					else if (IS_COMMAND('S','L','P'))
					{
						bExit = TRUE;

						switch (StageSelectLoop(&z))
						{
							case enum_ESCRETURN_exit:
								return enum_ESCRETURN_exit;

							case enum_ESCRETURN_restart:
								return enum_ESCRETURN_restart;
						}

						JumpTextScript(z);
						g_GameFlags &= ~3;
					}
					else if (IS_COMMAND('D','N','P'))
					{
						z = GetTextScriptNo(4);
						DeleteNpCharEvent(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('D','N','A'))
					{
						z = GetTextScriptNo(4);
						DeleteNpCharCode(z, TRUE);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('B','O','A'))
					{
						z = GetTextScriptNo(4);
						SetBossCharActNo(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('C','N','P'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						z = GetTextScriptNo(14);
						ChangeNpCharByEvent(x, y, z);
						gTS.p_read += 18;
					}
					else if (IS_COMMAND('A','N','P'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						z = GetTextScriptNo(14);
						SetNpCharActionNo(x, y, z);
						gTS.p_read += 18;
					}
					else if (IS_COMMAND('I','N','P'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						z = GetTextScriptNo(14);
						ChangeCheckableNpCharByEvent(x, y, z);
						gTS.p_read += 18;
					}
					else if (IS_COMMAND('S','N','P'))
					{
						w = GetTextScriptNo(4);
						x = GetTextScriptNo(9);
						y = GetTextScriptNo(14);
						z = GetTextScriptNo(19);
						SetNpChar(w, x * 0x200 * 0x10, y * 0x200 * 0x10, 0, 0, z, NULL, 0x100);
						gTS.p_read += 23;
					}
					else if (IS_COMMAND('M','N','P'))
					{
						w = GetTextScriptNo(4);
						x = GetTextScriptNo(9);
						y = GetTextScriptNo(14);
						z = GetTextScriptNo(19);
						MoveNpChar(w, x * 0x200 * 0x10, y * 0x200 * 0x10, z);
						gTS.p_read += 23;
					}
					else if (IS_COMMAND('S','M','P'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						ShiftMapParts(x, y);
						gTS.p_read += 13;
					}
					else if (IS_COMMAND('C','M','P'))
					{
						x = GetTextScriptNo(4);
						y = GetTextScriptNo(9);
						z = GetTextScriptNo(14);
						ChangeMapParts(x, y, z);
						gTS.p_read += 18;
					}
					else if (IS_COMMAND('B','S','L'))
					{
						z = GetTextScriptNo(4);

						if (z != 0)
							StartBossLife(z);
						else
							StartBossLife2();

						gTS.p_read += 8;
					}
					else if (IS_COMMAND('M','Y','D'))
					{
						z = GetTextScriptNo(4);
						SetMyCharDirect(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('M','Y','B'))
					{
						z = GetTextScriptNo(4);
						BackStepMyChar(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('M','M','0'))
					{
						ZeroMyCharXMove();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('I','N','I'))
					{
						InitializeGame(gProfileId);
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('S','V','P'))
					{
						SaveProfile(gProfileId);
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('L','D','P'))
					{
						if (!LoadProfile(gProfileId))
							InitializeGame(gProfileId);
					}
					else if (IS_COMMAND('F','A','C'))
					{
						z = GetTextScriptNo(4);
						if (gTS.face != (signed char)z)	// Not sure why these casts are here, but they're needed for the same assembly code to be produced
						{
							gTS.face = (signed char)z;
							gTS.face_x = (WINDOW_WIDTH / 2 - 156) * 0x200;
						}
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('F','A','C'))	// Duplicate command
					{
						z = GetTextScriptNo(4);
						if (gTS.face != (signed char)z)
						{
							gTS.face = (signed char)z;
							gTS.face_x = (WINDOW_WIDTH / 2 - 156) * 0x200;
						}
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('G','I','T'))
					{
						z = GetTextScriptNo(4);
						gTS.item = z;
						gTS.item_y = WINDOW_HEIGHT - 112;
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('N','U','M'))
					{
						// This supports up to four different values, but only one is actually used (a second is used erroneously)
						z = GetTextScriptNo(4);
						SetNumberTextScript(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('C','R','E'))
					{
						g_GameFlags |= 8;
						StartCreditScript();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('S','I','L'))
					{
						z = GetTextScriptNo(4);
						SetCreditIllust(z);
						gTS.p_read += 8;
					}
					else if (IS_COMMAND('C','I','L'))
					{
						CutCreditIllust();
						gTS.p_read += 4;
					}
					else if (IS_COMMAND('X','X','1'))
					{
						bExit = TRUE;
						z = GetTextScriptNo(4);

						switch (Scene_DownIsland(z))
						{
							case enum_ESCRETURN_exit:
								return enum_ESCRETURN_exit;

							case enum_ESCRETURN_restart:
								return enum_ESCRETURN_restart;
						}

						gTS.p_read += 8;
					}
					else if (IS_COMMAND('E','S','C'))
					{
						return enum_ESCRETURN_restart;
					}
					else if (IS_COMMAND('A','C','H'))
					{
						z = GetTextScriptNo(4);
						gTS.p_read += 8;
					}
					else
					{
						char str_0[0x40];
						sprintf(str_0, "Unknown code:<%c%c%c", gTS.p_read[1], gTS.p_read[2], gTS.p_read[3]);
						Backend_ShowMessageBox("Error", str_0);
						return enum_ESCRETURN_exit;
					}
				}
				else
				{
					if (*gTS.p_read == '\r')
					{
						// Go to new-line
						gTS.p_read += 2;
						
						if (gTS.flags & 1)
						{
							++gTS.line;
							CheckNewLine();
						}
					}
					else if (gTS.flags & 0x10)
					{
						// SAT/CAT/TUR printing
						char *readp = gTS.p_read;
						
						// Break if reaches command, or new-line
						while (*readp != '<' && *readp != '\r')
						{
							// Skip if SHIFT-JIS
							if (*readp & 0x80)
								readp++;
							readp++;
						}

						// Get text to copy
						y = readp - gTS.p_read;
						memcpy(str, gTS.p_read, y);
						str[y] = '\0';

						// Print text
						strcpy(text[gTS.line % 4], str);

						// Check if should move to next line (prevent a memory overflow, come on guys, this isn't a leftover of pixel trying to make text wrapping)
						gTS.p_read += y;

						bExit = TRUE;
					}
					else
					{
						// Get text to print
						c[0] = *gTS.p_read;

						if (c[0] & 0x80)
						{
							c[1] = gTS.p_read[1];
							c[2] = '\0';
						}
						else
						{
							c[1] = '\0';
						}

						// Print text
						strcat(text[gTS.line % 4], c);
						PlaySoundObject(2, 1);
						gTS.wait_beam = 0;

						// Offset read and write positions
						if (c[0] & 0x80)
							gTS.p_read += 2;
						else
							gTS.p_read += 1;

						bExit = TRUE;
					}
				}
			}
			break;

		case 2: // NOD
			if (gKeyTrg & (gKeyOk | gKeyCancel))
				gTS.mode = 1;
			break;

		case 3: // NEW LINE
			for (i = 0; i < 4; ++i)
			{
				gTS.ypos_line[i] -= 4;

				if (gTS.ypos_line[i] == 0)
					gTS.mode = 1;

				if (gTS.ypos_line[i] == -16)
					gTS.ypos_line[i] = 48;
			}
			break;

		case 4: // WAI
			if (gTS.wait_next == 9999)
				break;

			if (gTS.wait != 9999)
				++gTS.wait;

			if (gTS.wait < gTS.wait_next)
				break;

			gTS.mode = 1;
			gTS.wait_beam = 0;
			break;

		case 5: // FAI/FAO
			if (GetFadeActive())
				break;

			gTS.mode = 1;
			gTS.wait_beam = 0;
			break;

		case 7: // WAS
			if ((gMC.flag & 8) == 0)
				break;

			gTS.mode = 1;
			gTS.wait_beam = 0;
			break;

		case 6: // YNJ
			if (gTS.wait < 16)
			{
				++gTS.wait;
			}
			else
			{
				// Select option
				if (gKeyTrg & gKeyOk)
				{
					PlaySoundObject(18, 1);

					if (gTS.select == 1)
					{
						JumpTextScript(gTS.next_event);
					}
					else
					{
						gTS.mode = 1;
						gTS.wait_beam = 0;
					}
				}
				// Yes
				else if (gKeyTrg & gKeyLeft)
				{
					gTS.select = 0;
					PlaySoundObject(1, 1);
				}
				// No
				else if (gKeyTrg & gKeyRight)
				{
					gTS.select = 1;
					PlaySoundObject(1, 1);
				}
			}
			break;
	}

	if (gTS.mode == 0)
		g_GameFlags &= ~4;
	else
		g_GameFlags |= 4;
	return enum_ESCRETURN_continue;
}

void RestoreTextScript(void)
{
	return;
}
