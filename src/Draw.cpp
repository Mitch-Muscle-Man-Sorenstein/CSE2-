#include "Draw.h"

#include <stddef.h>
#include <string.h>
#include <string>

#include "WindowsWrapper.h"

#include "Backends/Misc.h"
#include "Backends/Rendering.h"
#include "Bitmap.h"
#include "CommonDefines.h"
#include "Ending.h"
#include "Font.h"
#include "Generic.h"
#include "Main.h"
#include "MapName.h"
#include "TextScr.h"

#define DRAW_SCALE 2

typedef enum SurfaceType
{
	SURFACE_SOURCE_NONE = 1,
	SURFACE_SOURCE_RESOURCE,
	SURFACE_SOURCE_FILE
} SurfaceType;

RECT grcGame = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
RECT grcFull = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

static BOOL fullscreen;	// TODO - Not the original variable name

static RenderBackend_Surface *framebuffer;	// TODO - Not the original variable name

static RenderBackend_Surface *surf[SURFACE_ID_MAX];

static FontObject *font;	// TODO - Not the original variable name

// This doesn't exist in the Linux port, so none of these symbol names are accurate
static struct
{
	char name[20];
	unsigned int width;
	unsigned int height;
	SurfaceType type;
	BOOL bSystem;	// Basically a 'do not regenerate' flag
} surface_metadata[SURFACE_ID_MAX];

BOOL Flip_SystemTask(void)
{
	// TODO - Not the original variable names
	static const long double rate = (1000.0L / 60.0L);
	static long double timePrev;
	static unsigned long timeNow;

	while (TRUE)
	{
		if (!SystemTask())
			return FALSE;

		// Framerate limiter
		timeNow = Backend_GetTicks();

		if (timeNow >= timePrev + rate)
			break;

		Backend_Delay(1);
	}

	if (timeNow >= timePrev + 100.0L)
		timePrev = timeNow;	// If the timer is freakishly out of sync, panic and reset it, instead of spamming frames for who-knows how long
	else
		timePrev += rate;

	RenderBackend_DrawScreen();

	if (RestoreSurfaces())
	{
		RestoreStripper();
		RestoreMapName();
		RestoreTextScript();
	}

	return TRUE;
}

BOOL StartDirectDraw(const char *title, int width, int height, int lMagnification)
{
	memset(surface_metadata, 0, sizeof(surface_metadata));

	switch (lMagnification)
	{
		case 0:
		case 1:
			fullscreen = FALSE;
			break;
		case 2:
			fullscreen = TRUE;
			break;
	}

	framebuffer = RenderBackend_Init(title, width, height, fullscreen);

	if (framebuffer == NULL)
		return FALSE;

	return TRUE;
}

void EndDirectDraw(void)
{
	int i;

	// Release all surfaces
	for (i = 0; i < SURFACE_ID_MAX; ++i)
		ReleaseSurface((SurfaceID)i);

	framebuffer = NULL;

	RenderBackend_Deinit();
}

void ReleaseSurface(SurfaceID s)
{
	// Release the surface we want to release
	if (surf[s] != NULL)
	{
		RenderBackend_FreeSurface(surf[s]);
		surf[s] = NULL;
	}

	memset(&surface_metadata[s], 0, sizeof(surface_metadata[0]));
}

static BOOL ScaleAndUploadSurface(const unsigned char *image_buffer, int width, int height, SurfaceID surf_no)
{
	// IF YOU WANT TO ADD HD SPRITES, THIS IS THE CODE YOU SHOULD EDIT
	unsigned int pitch;
	unsigned char *pixels = RenderBackend_LockSurface(surf[surf_no], &pitch, width, height);

	if (pixels == NULL)
		return FALSE;

	if (1)
	{
		// Just copy the pixels the way they are
		for (int y = 0; y < height; ++y)
		{
			const unsigned char *src_row = &image_buffer[y * width * 3];
			unsigned char *dst_row = &pixels[y * pitch];

			memcpy(dst_row, src_row, width * 3);
		}
	}
	else
	{
		// Upscale the bitmap to the game's internal resolution
		for (int y = 0; y < height; ++y)
		{
			const unsigned char *src_row = &image_buffer[y * width * 3];
			unsigned char *dst_row = &pixels[y * pitch * DRAW_SCALE];

			const unsigned char *src_ptr = src_row;
			unsigned char *dst_ptr = dst_row;

			for (int x = 0; x < width; ++x)
			{
				for (int i = 0; i < DRAW_SCALE; ++i)
				{
					*dst_ptr++ = src_ptr[0];
					*dst_ptr++ = src_ptr[1];
					*dst_ptr++ = src_ptr[2];
				}

				src_ptr += 3;
			}

			for (int i = 1; i < DRAW_SCALE; ++i)
				memcpy(dst_row + i * pitch, dst_row, width * DRAW_SCALE * 3);
		}
	}

	RenderBackend_UnlockSurface(surf[surf_no], width, height);

	return TRUE;
}

// TODO - Inaccurate stack frame
BOOL MakeSurface_Resource(const char *name, SurfaceID surf_no)
{
	return MakeSurface_File(name, surf_no);
}

// TODO - Inaccurate stack frame
BOOL MakeSurface_File(const char *name, SurfaceID surf_no)
{
	std::string path = gDataPath + '/' + name + ".bmp";

	if (surf_no >= SURFACE_ID_MAX)
	{
		ErrorLog("surface no", surf_no);
		return FALSE;
	}

	if (surf[surf_no] != NULL)
	{
		ErrorLog("existing", surf_no);
		return FALSE;
	}

	unsigned int width, height;
	unsigned char *image_buffer = DecodeBitmapFromFile(path.c_str(), &width, &height);

	if (image_buffer == NULL)
	{
		ErrorLog(path.c_str(), 1);
		return FALSE;
	}

	surf[surf_no] = RenderBackend_CreateSurface(width, height, false);

	if (surf[surf_no] == NULL)
	{
		FreeBitmap(image_buffer);
		return FALSE;
	}

	if (!ScaleAndUploadSurface(image_buffer, width, height, surf_no))
	{
		RenderBackend_FreeSurface(surf[surf_no]);
		FreeBitmap(image_buffer);
		return FALSE;
	}

	surface_metadata[surf_no].type = SURFACE_SOURCE_FILE;
	surface_metadata[surf_no].width = width;
	surface_metadata[surf_no].height = height;
	surface_metadata[surf_no].bSystem = FALSE;
	strcpy(surface_metadata[surf_no].name, name);
	FreeBitmap(image_buffer);

	return TRUE;
}

// TODO - Inaccurate stack frame
BOOL ReloadBitmap_Resource(const char *name, SurfaceID surf_no)
{
	return ReloadBitmap_File(name, surf_no);
}

// TODO - Inaccurate stack frame
BOOL ReloadBitmap_File(const char *name, SurfaceID surf_no)
{
	std::string path = gDataPath + '/' + name + ".bmp";
	
	if (surf_no >= SURFACE_ID_MAX)
	{
		ErrorLog("surface no", surf_no);
		return FALSE;
	}

	unsigned int width, height;
	unsigned char *image_buffer = DecodeBitmapFromFile(path.c_str(), &width, &height);

	if (image_buffer == NULL)
	{
		ErrorLog(path.c_str(), 1);
		return FALSE;
	}

	if (!ScaleAndUploadSurface(image_buffer, width, height, surf_no))
	{
		FreeBitmap(image_buffer);
		return FALSE;
	}

	FreeBitmap(image_buffer);
	surface_metadata[surf_no].type = SURFACE_SOURCE_FILE;
	strcpy(surface_metadata[surf_no].name, name);

	return TRUE;
}

// TODO - Inaccurate stack frame
BOOL MakeSurface_Generic(int bxsize, int bysize, SurfaceID surf_no, BOOL bSystem)
{
#ifdef FIX_BUGS
	if (surf_no >= SURFACE_ID_MAX)
#else
	if (surf_no > SURFACE_ID_MAX)	// OOPS (should be '>=')
#endif
		return FALSE;

	if (surf[surf_no] != NULL)
		return FALSE;

	surf[surf_no] = RenderBackend_CreateSurface(bxsize * DRAW_SCALE, bysize * DRAW_SCALE, true);

	if (surf[surf_no] == NULL)
		return FALSE;

	surface_metadata[surf_no].type = SURFACE_SOURCE_NONE;
	surface_metadata[surf_no].width = bxsize;
	surface_metadata[surf_no].height = bysize;

	if (bSystem)
		surface_metadata[surf_no].bSystem = TRUE;
	else
		surface_metadata[surf_no].bSystem = FALSE;

	strcpy(surface_metadata[surf_no].name, "generic");

	return TRUE;
}

void BackupSurface(SurfaceID surf_no, const RECT *rect)
{
	static RenderBackend_Rect rcSet;	// TODO - Not the original variable name
	rcSet.left = rect->left * DRAW_SCALE;
	rcSet.top = rect->top * DRAW_SCALE;
	rcSet.right = rect->right * DRAW_SCALE;
	rcSet.bottom = rect->bottom * DRAW_SCALE;

	// Do not draw invalid RECTs
	if (rcSet.right <= rcSet.left || rcSet.bottom <= rcSet.top)
		return;

	RenderBackend_Blit(framebuffer, &rcSet, surf[surf_no], rcSet.left, rcSet.top, FALSE);
}

void PutBitmap3(const RECT *rcView, int x, int y, const RECT *rect, SurfaceID surf_no) // Transparency
{
	static RenderBackend_Rect rcWork;

	rcWork.left = rect->left;
	rcWork.top = rect->top;
	rcWork.right = rect->right;
	rcWork.bottom = rect->bottom;

	if (x + rect->right - rect->left > rcView->right)
		rcWork.right -= (x + rect->right - rect->left) - rcView->right;

	if (x < rcView->left)
	{
		rcWork.left += rcView->left - x;
		x = rcView->left;
	}

	if (y + rect->bottom - rect->top > rcView->bottom)
		rcWork.bottom -= (y + rect->bottom - rect->top) - rcView->bottom;

	if (y < rcView->top)
	{
		rcWork.top += rcView->top - y;
		y = rcView->top;
	}

	rcWork.left *= DRAW_SCALE;
	rcWork.top *= DRAW_SCALE;
	rcWork.right *= DRAW_SCALE;
	rcWork.bottom *= DRAW_SCALE;

	// Do not draw invalid RECTs
	if (rcWork.right <= rcWork.left || rcWork.bottom <= rcWork.top)
		return;

	RenderBackend_Blit(surf[surf_no], &rcWork, framebuffer, x * DRAW_SCALE, y * DRAW_SCALE, TRUE);
}

void PutBitmap4(const RECT *rcView, int x, int y, const RECT *rect, SurfaceID surf_no) // No Transparency
{
	static RenderBackend_Rect rcWork;

	rcWork.left = rect->left;
	rcWork.top = rect->top;
	rcWork.right = rect->right;
	rcWork.bottom = rect->bottom;

	if (x + rect->right - rect->left > rcView->right)
		rcWork.right -= (x + rect->right - rect->left) - rcView->right;

	if (x < rcView->left)
	{
		rcWork.left += rcView->left - x;
		x = rcView->left;
	}

	if (y + rect->bottom - rect->top > rcView->bottom)
		rcWork.bottom -= (y + rect->bottom - rect->top) - rcView->bottom;

	if (y < rcView->top)
	{
		rcWork.top += rcView->top - y;
		y = rcView->top;
	}

	rcWork.left *= DRAW_SCALE;
	rcWork.top *= DRAW_SCALE;
	rcWork.right *= DRAW_SCALE;
	rcWork.bottom *= DRAW_SCALE;

	// Do not draw invalid RECTs
	if (rcWork.right <= rcWork.left || rcWork.bottom <= rcWork.top)
		return;

	RenderBackend_Blit(surf[surf_no], &rcWork, framebuffer, x * DRAW_SCALE, y * DRAW_SCALE, FALSE);
}

void Surface2Surface(int x, int y, const RECT *rect, int to, int from)
{
	static RenderBackend_Rect rcWork;

	rcWork.left = rect->left * DRAW_SCALE;
	rcWork.top = rect->top * DRAW_SCALE;
	rcWork.right = rect->right * DRAW_SCALE;
	rcWork.bottom = rect->bottom * DRAW_SCALE;

	// Do not draw invalid RECTs
	if (rcWork.right <= rcWork.left || rcWork.bottom <= rcWork.top)
		return;

	RenderBackend_Blit(surf[from], &rcWork, surf[to], x * DRAW_SCALE, y * DRAW_SCALE, TRUE);
}

unsigned long GetCortBoxColor(unsigned long col)
{
	// Comes in 00BBGGRR, goes out 00BBGGRR
	return col;
}

void CortBox(const RECT *rect, unsigned long col)
{
	static RenderBackend_Rect rcSet;	// TODO - Not the original variable name
	rcSet.left = rect->left * DRAW_SCALE;
	rcSet.top = rect->top * DRAW_SCALE;
	rcSet.right = rect->right * DRAW_SCALE;
	rcSet.bottom = rect->bottom * DRAW_SCALE;

	const unsigned char red = col & 0xFF;
	const unsigned char green = (col >> 8) & 0xFF;
	const unsigned char blue = (col >> 16) & 0xFF;

	// Do not draw invalid RECTs
	if (rcSet.right <= rcSet.left || rcSet.bottom <= rcSet.top)
		return;

	RenderBackend_ColourFill(framebuffer, &rcSet, red, green, blue);
}

void CortBox2(const RECT *rect, unsigned long col, SurfaceID surf_no)
{
	static RenderBackend_Rect rcSet;	// TODO - Not the original variable name
	rcSet.left = rect->left * DRAW_SCALE;
	rcSet.top = rect->top * DRAW_SCALE;
	rcSet.right = rect->right * DRAW_SCALE;
	rcSet.bottom = rect->bottom * DRAW_SCALE;

	surface_metadata[surf_no].type = SURFACE_SOURCE_NONE;

	const unsigned char red = col & 0xFF;
	const unsigned char green = (col >> 8) & 0xFF;
	const unsigned char blue = (col >> 16) & 0xFF;

	// Do not draw invalid RECTs
	if (rcSet.right <= rcSet.left || rcSet.bottom <= rcSet.top)
		return;

	RenderBackend_ColourFill(surf[surf_no], &rcSet, red, green, blue);
}

// Dummied-out log function
// According to the Mac port, its name really is just "out".
BOOL out(int unknown)
{
	char unknown2[0x100];
	int unknown3;
	int unknown4;
	int unknown5;

	(void)unknown;
	(void)unknown2;
	(void)unknown3;
	(void)unknown4;
	(void)unknown5;

	return TRUE;
}

// TODO - Probably not the original variable name (this is an educated guess)
int RestoreSurfaces(void)
{
	int s;
	RECT rect;
	int surfaces_regenerated = 0;

	if (framebuffer == NULL)
		return surfaces_regenerated;

	if (RenderBackend_IsSurfaceLost(framebuffer))
	{
		++surfaces_regenerated;
		RenderBackend_RestoreSurface(framebuffer);
		out('f');	// 'f' for 'frontbuffer' (or, in this branch's case, 'framebuffer')
	}

	for (s = 0; s < SURFACE_ID_MAX; ++s)
	{
		if (surf[s] != NULL)
		{
			if (RenderBackend_IsSurfaceLost(surf[s]))
			{
				++surfaces_regenerated;
				RenderBackend_RestoreSurface(surf[s]);
				out('0' + s);	// The number of the surface lost

				if (!surface_metadata[s].bSystem)
				{
					switch (surface_metadata[s].type)
					{
						case SURFACE_SOURCE_NONE:
							rect.left = 0;
							rect.top = 0;
							rect.right = surface_metadata[s].width;
							rect.bottom = surface_metadata[s].height;
							CortBox2(&rect, 0, (SurfaceID)s);
							break;

						case SURFACE_SOURCE_RESOURCE:
							ReloadBitmap_Resource(surface_metadata[s].name, (SurfaceID)s);
							break;

						case SURFACE_SOURCE_FILE:
							ReloadBitmap_File(surface_metadata[s].name, (SurfaceID)s);
							break;
					}
				}
			}
		}
	}

	return surfaces_regenerated;
}

// TODO - Inaccurate stack frame
void InitTextObject(const char *name)
{
	std::string path = gDataPath + "/" + name;

	// Get font size
	unsigned int width, height;

	switch (DRAW_SCALE)
	{
#ifdef JAPANESE
		case 1:
			height = 12;
			width = 12;
			break;

		case 2:
			height = 20;
			width = 20;
			break;
#else
		case 1:
			height = 10;
			width = 9;
			break;

		case 2:
			height = 18;
			width = 17;
			break;
#endif
	}

	font = LoadFont(path.c_str(), width, height);
}

unsigned int GetTextWidth(const char *text)
{
	return GetFontTextWidth(font, text) / DRAW_SCALE;
}

void PutText(int x, int y, const char *text, unsigned long color)
{
	DrawText(font, framebuffer, x * DRAW_SCALE, y * DRAW_SCALE, color, text);
}

void PutText2(int x, int y, const char *text, unsigned long color, SurfaceID surf_no)
{
	DrawText(font, surf[surf_no], x * DRAW_SCALE, y * DRAW_SCALE, color, text);
}

void EndTextObject(void)
{
	UnloadFont(font);
}
