#include "Title.h"
#include "CommonDefines.h"
#include "Game.h"
#include "MyChar.h"

void Title_PutLife(int x, int y, int life, int max_life)
{
	RECT rcCase = {0, 40, 232, 48};
	RECT rcLife = {0, 24, 232, 32};
	
	// Draw bar
	rcCase.right = 64;
	rcLife.right = ((life * 40) / max_life) - 1;

	PutBitmap3(&grcFull, x, y, &rcCase, SURFACE_ID_TEXT_BOX);
	PutBitmap3(&grcFull, x + 24, y, &rcLife, SURFACE_ID_TEXT_BOX);
	PutNumber4(x - 8, y, life, FALSE);
}

void Title_PutSave(PROFILE *profile, int y, BOOL selected)
{
	//Draw box
	RECT rcSave = {(WINDOW_WIDTH - 200) / 2, y, (WINDOW_WIDTH + 200) / 2, y + 42};
	Title_PutBox(&rcSave, selected);
	
	if (profile != nullptr)
	{
		//Draw health
		Title_PutLife((WINDOW_WIDTH / 2) - 90, y + 10, profile->life, profile->max_life);
		
		//Draw arms
		int code;
		for (int i = 0; (code = profile->arms[i].code) != 0; i++)
		{
			RECT rcArms = {code * 16, 0, (code + 1) * 16, 16};
			PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) - 18 + i * 16, y + 3, &rcArms, SURFACE_ID_ARMS_IMAGE);
		}
		
		//Draw saved time and saved map
		PutText((WINDOW_WIDTH / 2) - 90, y + 21, profile->time, 0xFFFFFF);
		PutText((WINDOW_WIDTH / 2) - 90, y + 31, gTMT[profile->stage].name, 0xFFFFFF);
		
		//Draw Quote
		static const MYCHAR_Costume difficulty_costumes[] = {
			MCC_Easy,
			MCC_Normal,
			MCC_Hard,
		};
		
		RECT rcQuote = {0, 0, 16, 16};
		if (profile->equip & EQUIP_MIMIGA_MASK)
		{
			rcQuote.top += 32;
			rcQuote.bottom += 32;
		}
		rcQuote.top += difficulty_costumes[profile->difficulty] * 64;
		rcQuote.bottom += difficulty_costumes[profile->difficulty] * 64;
		
		PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) + 36, y + 23, &rcQuote, SURFACE_ID_MY_CHAR);
	}
	else
	{
		//Draw no save default
		Title_PutLife((WINDOW_WIDTH / 2) - 90, y + 10, 3, 3);
		PutText((WINDOW_WIDTH / 2) - 90, y + 21, "New", 0xFFFFFF);
	}
}

void Title_PutCenterText(int x, int y, const char *text, unsigned long color)
{
	PutText(x - GetTextWidth(text) / 2, y, text, color);
}

#define DEF_TPB_RECT(name, l, t, r, b) \
	static const RECT name##ns = {l,t,r,b}; \
	static const RECT name##ys = {l+16,t,r+16,b}; \
	RECT name = selected ? name##ys : name##ns;

void Title_PutBox(const RECT *rect, BOOL selected)
{
	//Box framerects
	DEF_TPB_RECT(rcTL, 0, 0,  8,  8)
	DEF_TPB_RECT(rcTR, 8, 0, 16,  8)
	DEF_TPB_RECT(rcBR, 8, 8, 16, 16)
	DEF_TPB_RECT(rcBL, 0, 8,  8, 16)
	
	DEF_TPB_RECT(rcLE, 0, 4,  8, 12)
	DEF_TPB_RECT(rcTE, 4, 0, 12,  8)
	DEF_TPB_RECT(rcRE, 8, 4, 16, 12)
	DEF_TPB_RECT(rcBE, 4, 8, 12, 16)
	
	DEF_TPB_RECT(rcMD, 4, 4, 12, 12)
	
	//Clip rects
	RECT boxM = {rect->left + 8, rect->top + 8, rect->right - 8, rect->bottom - 8};
	RECT boxH = {0, rect->top + 8, WINDOW_WIDTH, rect->bottom - 8};
	RECT boxV = {rect->left + 8, 0, rect->right - 8, WINDOW_HEIGHT};
	
	//Draw box NOW
	for (int x = rect->left + 8; x < rect->right - 8; x += 8)
		for (int y = rect->top + 8; y < rect->bottom - 8; y += 8)
			PutBitmap3(&boxM, x, y, &rcMD, SURFACE_ID_UI);
	
	for (int x = rect->left + 8; x < rect->right - 8; x += 8)
	{
		PutBitmap3(&boxV, x, rect->top, &rcTE, SURFACE_ID_UI);
		PutBitmap3(&boxV, x, rect->bottom - 8, &rcBE, SURFACE_ID_UI);
	}
	
	for (int y = rect->top + 8; y < rect->bottom - 8; y += 8)
	{
		PutBitmap3(&boxH, rect->left, y, &rcLE, SURFACE_ID_UI);
		PutBitmap3(&boxH, rect->right - 8, y, &rcRE, SURFACE_ID_UI);
	}
	
	PutBitmap3(&grcFull, rect->left, rect->top, &rcTL, SURFACE_ID_UI);
	PutBitmap3(&grcFull, rect->right - 8, rect->top, &rcTR, SURFACE_ID_UI);
	PutBitmap3(&grcFull, rect->right - 8, rect->bottom - 8, &rcBR, SURFACE_ID_UI);
	PutBitmap3(&grcFull, rect->left, rect->bottom - 8, &rcBL, SURFACE_ID_UI);
}

void Title_PutTextBox(int x, int y, const char *text, BOOL selected)
{
	RECT box = {x - 14, y - 8, x + GetTextWidth(text) + 14, y + 16};
	Title_PutBox(&box, selected);
	PutText(x, y, text, 0xFFFFFF);
}

void Title_PutTextBox2(int x, int y, const char *text, BOOL selected)
{
	RECT box = {x - 8, y - 8, x + GetTextWidth(text) + 8, y + 16};
	Title_PutBox(&box, selected);
	PutText(x, y, text, 0xFFFFFF);
}

void Title_PutCenterTextBox(int x, int y, const char *text, BOOL selected)
{
	Title_PutTextBox(x - GetTextWidth(text) / 2, y, text, selected);
}
