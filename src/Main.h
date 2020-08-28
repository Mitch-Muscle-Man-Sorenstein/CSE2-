#pragma once

#include "WindowsWrapper.h"

enum GameSeason
{
	GS_None,
	GS_Pixel,
	GS_Halloween,
	GS_Christmas,
};

extern GameSeason g_GameSeason;

extern BOOL bFullscreen;
extern BOOL gbUseJoystick;

extern int gJoystickButtonTable[8];

extern BOOL gbUseJoystick;

void PutFramePerSecound(void);

void InactiveWindow(void);
void ActiveWindow(void);

BOOL SystemTask(void);
