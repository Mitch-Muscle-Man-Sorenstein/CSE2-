#include "Back.h"

#include <stddef.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "WindowsWrapper.h"

#include "CommonDefines.h"
#include "Draw.h"
#include "File.h"
#include "Filesystem.h"

BACK gBack;
int gWaterY;

// TODO - Another function that has an incorrect stack frame
BOOL InitBack(std::string fName, int type)
{
	//Determine if original or new graphics should be used
	std::string path;
	BOOL is_original;
	
	if (gUseOriginalGraphics && FileExists(path = FindFile(FSS_Mod, std::string("ogph/") + fName + ".bmp")))
	{
		//Original graphics is on and an original graphics file was found
		is_original = TRUE;
	}
	else if (FileExists(path = FindFile(FSS_Mod, fName + ".bmp")))
	{
		//New graphics file was found
		is_original = FALSE;
	}
	else
	{
		//No file was found
		return FALSE;
	}
	
	//Open bitmap file and read dimensions
	FILE *fp = fopen(path.c_str(), "rb");
	if (fp == NULL)
		return FALSE;

	if (fgetc(fp) != 'B' || fgetc(fp) != 'M')
	{
		fclose(fp);
		return FALSE;
	}
	
	fseek(fp, 18, SEEK_SET);
	
	if (is_original)
	{
		gBack.partsW = File_ReadLE32(fp);
		gBack.partsH = File_ReadLE32(fp);
	}
	else
	{
		gBack.partsW = File_ReadLE32(fp) / DRAW_SCALE;
		gBack.partsH = File_ReadLE32(fp) / DRAW_SCALE;
	}
	
	fclose(fp);

	//Setup background and load texture
	gBack.type = type;
	gWaterY = 240 * 0x10 * 0x200;
	
	if (!ReloadBitmap_File(fName, SURFACE_ID_LEVEL_BACKGROUND))
		return FALSE;
	return TRUE;
}

void ActBack(void)
{
	switch (gBack.type)
	{
		case 5:
			gBack.fx += 6 * 0x200;
			break;

		case 6:
		case 7:
			++gBack.fx;
			gBack.fx %= 640;
			break;
		case 8:
			if (--gBack.fx < 0)
				gBack.fx = 640;
			break;
	}
}

void PutBack_Strip(int top, int bottom, float fx)
{
	RECT rect = {0, top, 320, bottom};
	for (float x = -std::fmod(fx, 320); x < WINDOW_WIDTH; x += 320)
		PutBitmap4(&grcGame, x, top, &rect, SURFACE_ID_LEVEL_BACKGROUND);
}

void PutBack(int fx, int fy)
{
	float x, y;
	RECT rect = {0, 0, gBack.partsW, gBack.partsH};

	switch (gBack.type)
	{
		case 0:
			for (y = 0; y < WINDOW_HEIGHT; y += gBack.partsH)
				for (x = 0; x < WINDOW_WIDTH; x += gBack.partsW)
					PutBitmap4(&grcGame, x, y, &rect, SURFACE_ID_LEVEL_BACKGROUND);

			break;

		case 1:
			for (y = -std::fmod(SubpixelToScreen(fy / 2), gBack.partsH); y < WINDOW_HEIGHT; y += gBack.partsH)
				for (x = -std::fmod(SubpixelToScreen(fx / 2), gBack.partsW); x < WINDOW_WIDTH; x += gBack.partsW)
					PutBitmap4(&grcGame, x, y, &rect, SURFACE_ID_LEVEL_BACKGROUND);

			break;

		case 2:
			for (y = -std::fmod(SubpixelToScreen(fy), gBack.partsH); y < WINDOW_HEIGHT; y += gBack.partsH)
				for (x = -std::fmod(SubpixelToScreen(fx), gBack.partsW); x < WINDOW_WIDTH; x += gBack.partsW)
					PutBitmap4(&grcGame, x, y, &rect, SURFACE_ID_LEVEL_BACKGROUND);

			break;

		case 5:
			for (y = 0; y < WINDOW_HEIGHT; y += gBack.partsH)
				for (x = -std::fmod(SubpixelToScreen(gBack.fx), gBack.partsW); x < WINDOW_WIDTH; x += gBack.partsW)
					PutBitmap4(&grcGame, x, y, &rect, SURFACE_ID_LEVEL_BACKGROUND);

			break;

		case 6:
		case 7:
		case 8:
			rect.top = 0;
			rect.bottom = 88;
			rect.left = 0;
			rect.right = gBack.partsW;
			PutBitmap4(&grcGame, 0, 0, &rect, SURFACE_ID_LEVEL_BACKGROUND);

			PutBack_Strip(88, 123, gBack.fx / 2);
			PutBack_Strip(123, 146, gBack.fx);
			PutBack_Strip(146, 176, gBack.fx * 2);
			PutBack_Strip(176, 240, gBack.fx * 4);
			break;
	}
}

void PutFront(int fx, int fy)
{
	float xpos, ypos;

	RECT rcWater[2] = {{0, 0, 32, 16}, {0, 16, 32, 48}};

	float x, y;
	float x_1, x_2;
	float y_1, y_2;

	switch (gBack.type)
	{
		case 3:
			x_1 = (float)fx / (32 * 0x200);
			x_2 = x_1 + (((WINDOW_WIDTH + (32 - 1)) / 32) + 1);
			y_1 = 0;
			y_2 = y_1 + 32;

			for (y = y_1; y < y_2; ++y)
			{
				ypos = SubpixelToScreen(y * 32 * 0x200) - SubpixelToScreen(fy) + SubpixelToScreen(gWaterY);

				if (ypos < -32)
					continue;

				if (ypos > WINDOW_HEIGHT)
					break;

				for (x = x_1; x < x_2; ++x)
				{
					xpos = SubpixelToScreen(x * 32 * 0x200) - SubpixelToScreen(fx);
					PutBitmap3(&grcGame, xpos, ypos, &rcWater[1], SURFACE_ID_LEVEL_BACKGROUND);
					if (y == 0)
						PutBitmap3(&grcGame, xpos, ypos, &rcWater[0], SURFACE_ID_LEVEL_BACKGROUND);
				}
			}

			break;
	}
}
