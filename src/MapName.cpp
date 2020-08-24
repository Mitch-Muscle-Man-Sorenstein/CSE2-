#include "MapName.h"

#include <string.h>

#include "CommonDefines.h"
#include "Draw.h"
#include "WindowsWrapper.h"

MAP_NAME gMapName;

void ReadyMapName(const char *str)
{
	// Copy map's name to the global map name
	gMapName.flag = FALSE;
	gMapName.wait = 0;
	strcpy(gMapName.name, str);
}

void PutMapName(BOOL bMini)
{
	if (bMini)
	{
		// Map system
		RECT rcBack;
		rcBack.left = 0;
		rcBack.right = WINDOW_WIDTH;
		rcBack.top = 7;
		rcBack.bottom = 24;

		CortBox(&rcBack, 0x000000);
		PutText((WINDOW_WIDTH / 2) - GetTextWidth(gMapName.name) / 2, 11, gMapName.name, RGB(0x11, 0x00, 0x22));
		PutText((WINDOW_WIDTH / 2) - GetTextWidth(gMapName.name) / 2, 10, gMapName.name, RGB(0xFF, 0xFF, 0xFE));
	}
	else if (gMapName.flag)
	{
		// MNA
		PutText((WINDOW_WIDTH / 2) - GetTextWidth(gMapName.name) / 2, (WINDOW_HEIGHT / 2) - 40 + 1, gMapName.name, RGB(0x11, 0x00, 0x22));
		PutText((WINDOW_WIDTH / 2) - GetTextWidth(gMapName.name) / 2, (WINDOW_HEIGHT / 2) - 40, gMapName.name, RGB(0xFF, 0xFF, 0xFE));
		if (++gMapName.wait > 160)
			gMapName.flag = FALSE;
	}
}

void StartMapName(void)
{
	gMapName.flag = TRUE;
	gMapName.wait = 0;
}

void RestoreMapName(void)
{
	
}
