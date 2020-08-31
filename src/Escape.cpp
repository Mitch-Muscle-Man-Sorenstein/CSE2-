#include "Escape.h"

#include "WindowsWrapper.h"

#include "CommonDefines.h"
#include "Draw.h"
#include "KeyControl.h"
#include "Main.h"
#include "Sound.h"

//Escape constants
#define BOX_FRAMES	(12)

#define YN_FRAMES	(6)
#define YN_FRAME	(BOX_FRAMES - YN_FRAMES)

#define SEL_FRAME	(YN_FRAME + YN_FRAMES)

//Escape drawing
void Escape_PutCenterText(int x, int y, const char *text, unsigned long color)
{
	PutText(x - GetTextWidth(text) / 2, y, text, color);
}

void Escape_PutBox(const RECT *rect)
{
	//Box framerects
	RECT rcTL = {  0,  0,   8,  8};
	RECT rcTR = {236,  0, 244,  8};
	RECT rcBR = {236, 16, 244, 24};
	RECT rcBL = {  0, 16,   8, 24};
	
	RECT rcLE = {  0,  8,   8, 16};
	RECT rcTE = {  8,  0, 236,  8};
	RECT rcRE = {236,  8, 244, 16};
	RECT rcBE = {  8, 16, 236, 24};
	
	RECT rcMD = {8, 8, 236, 16};
	
	//Clip rects
	RECT boxM = {rect->left + 8, rect->top + 8, rect->right - 8, rect->bottom - 8};
	RECT boxH = {0, rect->top + 8, WINDOW_WIDTH, rect->bottom - 8};
	RECT boxV = {rect->left + 8, 0, rect->right - 8, WINDOW_HEIGHT};
	
	//Draw box NOW
	for (int x = rect->left + 8; x < rect->right - 8; x += 228)
		for (int y = rect->top + 8; y < rect->bottom - 8; y += 8)
			PutBitmap3(&boxM, x, y, &rcMD, SURFACE_ID_TEXT_BOX);
	
	for (int x = rect->left + 8; x < rect->right - 8; x += 228)
	{
		PutBitmap3(&boxV, x, rect->top, &rcTE, SURFACE_ID_TEXT_BOX);
		PutBitmap3(&boxV, x, rect->bottom - 8, &rcBE, SURFACE_ID_TEXT_BOX);
	}
	
	for (int y = rect->top + 8; y < rect->bottom - 8; y += 8)
	{
		PutBitmap3(&boxH, rect->left, y, &rcLE, SURFACE_ID_TEXT_BOX);
		PutBitmap3(&boxH, rect->right - 8, y, &rcRE, SURFACE_ID_TEXT_BOX);
	}
	
	PutBitmap3(&grcFull, rect->left, rect->top, &rcTL, SURFACE_ID_TEXT_BOX);
	PutBitmap3(&grcFull, rect->right - 8, rect->top, &rcTR, SURFACE_ID_TEXT_BOX);
	PutBitmap3(&grcFull, rect->right - 8, rect->bottom - 8, &rcBR, SURFACE_ID_TEXT_BOX);
	PutBitmap3(&grcFull, rect->left, rect->bottom - 8, &rcBL, SURFACE_ID_TEXT_BOX);
}

//Escape loop
int Call_Escape(void)
{
	//Initialize state
	int frame = 0;
	int option = 1; //No
	
	while (1)
	{
		// Get pressed keys
		GetTrg();
		
		//Play YNJ sound once it starts appearing
		if (frame == YN_FRAME)
			PlaySoundObject(5, 1);
		
		//Exit if escape is pressed again
		if (gKeyTrg & KEY_ESCAPE)
		{
			gKeyInternal &= ~KEY_ESCAPE;
			gKeyTrg = 0;
			return enum_ESCRETURN_continue;
		}
		
		if (frame >= SEL_FRAME)
		{
			//Move Yes / No cursor
			if (gKeyTrg & (gKeyLeft | gKeyRight))
			{
				PlaySoundObject(1, 1);
				if (gKeyTrg & gKeyLeft)
					option = 0;
				if (gKeyTrg & gKeyRight)
					option = 1;
			}
			
			//Select option when Ok is pressed
			if (gKeyTrg & gKeyOk)
			{
				PlaySoundObject(18, 1);
				gKeyTrg &= ~gKeyOk;
				return option ? enum_ESCRETURN_continue : enum_ESCRETURN_restart;
			}
		}
		
		//Draw screen capture
		PutBitmap4(&grcFull, 0, 0, &grcFull, SURFACE_ID_SCREEN_GRAB);
		
		//Draw box
		RECT x = {(WINDOW_WIDTH - 68) / 2, 116, (WINDOW_WIDTH + 68) / 2, 140};
		RECT y = {(WINDOW_WIDTH - 236) / 2, 108, (WINDOW_WIDTH + 236) / 2, 148};
		
		RECT z;
		if (frame < BOX_FRAMES)
		{
			//Interpolate between starting and ending rect
			z.left   = x.left   + (y.left  -  x.left  ) * frame / BOX_FRAMES;
			z.top    = x.top    + (y.top -    x.top   ) * frame / BOX_FRAMES;
			z.right  = x.right  + (y.right -  x.right ) * frame / BOX_FRAMES;
			z.bottom = x.bottom + (y.bottom - x.bottom) * frame / BOX_FRAMES;
		}
		else
		{
			//Use ending rect
			z.left = y.left;
			z.top = y.top;
			z.right = y.right;
			z.bottom = y.bottom;
		}
		
		Escape_PutBox(&z);
		
		//Draw warning text
		if (frame >= BOX_FRAMES)
		{
			static const unsigned long red_color = GetCortBoxColor(0x4040FF);
			Escape_PutCenterText(WINDOW_WIDTH / 2, 116, "Quit to Main Menu?  Really??", 0xFFFFFF);
			Escape_PutCenterText(WINDOW_WIDTH / 2, 132, "Any unsaved progress will be lost!", red_color);
		}
		
		//Draw Yes / No box
		RECT rect_yesno = {152, 48, 244, 80};
		RECT rect_cur = {112, 88, 128, 104};

		if (frame >= YN_FRAME)
		{
			//Get position offset
			int off = YN_FRAMES - (frame - YN_FRAME);
			if (off > 0)
				off = off * 16 / YN_FRAMES;
			else
				off = 0;
			
			//Draw box and cursor
			PutBitmap3(&grcFull, (WINDOW_WIDTH / 2) + 74, 86 + off, &rect_yesno, SURFACE_ID_TEXT_BOX);
			if (frame >= SEL_FRAME)
				PutBitmap3(&grcFull, (option * 41) + (WINDOW_WIDTH / 2) + 69, 96, &rect_cur, SURFACE_ID_TEXT_BOX);
		}
		
		//Render frame
		if (frame < SEL_FRAME)
			frame++;
		PutFramePerSecound();
		if (!Flip_SystemTask())
		{
			// Quit if window is closed
			gKeyTrg = 0;
			return enum_ESCRETURN_exit;
		}
	}

	return enum_ESCRETURN_continue;
}
