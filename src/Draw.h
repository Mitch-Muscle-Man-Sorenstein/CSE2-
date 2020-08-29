#pragma once
#include "WindowsWrapper.h"
#include <string>

#define DRAW_SCALE 2

extern BOOL gUseOriginalGraphics;

extern RECT grcGame;
extern RECT grcFull;

typedef enum SurfaceID
{
	SURFACE_ID_TITLE = 0,
	SURFACE_ID_NICALIS = 1,
	SURFACE_ID_LEVEL_TILESET = 2,
	SURFACE_ID_FADE = 6,
	SURFACE_ID_ITEM_IMAGE = 8,
	SURFACE_ID_MAP = 9,
	SURFACE_ID_SCREEN_GRAB = 10,
	SURFACE_ID_ARMS = 11,
	SURFACE_ID_ARMS_IMAGE = 12,
	SURFACE_ID_UI = 13,
	SURFACE_ID_STAGE_ITEM = 14,
	SURFACE_ID_LOADING = 15,
	SURFACE_ID_MY_CHAR = 16,
	SURFACE_ID_BULLET = 17,
	SURFACE_ID_CARET = 19,
	SURFACE_ID_NPC_SYM = 20,
	SURFACE_ID_LEVEL_SPRITESET_1 = 21,
	SURFACE_ID_LEVEL_SPRITESET_2 = 22,
	SURFACE_ID_NPC_REGU = 23,
	SURFACE_ID_TEXT_BOX = 26,
	SURFACE_ID_FACE = 27,
	SURFACE_ID_LEVEL_BACKGROUND = 28,
	SURFACE_ID_VALUE_VIEW = 29,
	//SURFACE_ID_TEXT_LINE1 = 30,
	//SURFACE_ID_TEXT_LINE2 = 31,
	//SURFACE_ID_TEXT_LINE3 = 32,
	//SURFACE_ID_TEXT_LINE4 = 33,
	//SURFACE_ID_TEXT_LINE5 = 34,
	//SURFACE_ID_CREDIT_CAST = 35,
	SURFACE_ID_CREDITS_IMAGE = 36,
	SURFACE_ID_CASTS = 37,
	SURFACE_ID_MAX = 40
} SurfaceID;

BOOL Flip_SystemTask(void);
BOOL StartDirectDraw(const char *title, int width, int height, int lMagnification);
void EndDirectDraw(void);
void ReleaseSurface(SurfaceID s);
BOOL MakeSurface_Resource(std::string name, SurfaceID surf_no);
BOOL MakeSurface_File(std::string name, SurfaceID surf_no);
BOOL ReloadBitmap_Resource(std::string name, SurfaceID surf_no);
BOOL ReloadBitmap_File(std::string name, SurfaceID surf_no);
BOOL MakeSurface_Generic(int bxsize, int bysize, SurfaceID surf_no, BOOL bSystem);
void BackupSurface(SurfaceID surf_no, const RECT *rect);
void PutBitmap3(const RECT *rcView, int x, int y, const RECT *rect, SurfaceID surf_no);
void PutBitmap4(const RECT *rcView, int x, int y, const RECT *rect, SurfaceID surf_no);
void Surface2Surface(int x, int y, const RECT *rect, int to, int from);
unsigned long GetCortBoxColor(unsigned long col);
void CortBox(const RECT *rect, unsigned long col);
void CortBox2(const RECT *rect, unsigned long col, SurfaceID surf_no);
int RestoreSurfaces(void);
void InitTextObject(std::string font_name);
unsigned int GetTextWidth(const char *text);
void PutText(int x, int y, const char *text, unsigned long color);
void PutText2(int x, int y, const char *text, unsigned long color, SurfaceID surf_no);
void EndTextObject(void);
