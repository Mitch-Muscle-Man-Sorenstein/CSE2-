#pragma once

#include "WindowsWrapper.h"

extern signed char gMapping[0x80];

int MiniMapLoop(void);
BOOL IsMapping(void);
void StartMapping(void);
void SetMapping(int a);
