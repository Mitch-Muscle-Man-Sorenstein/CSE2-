#pragma once

#include "WindowsWrapper.h"

#include "PixTone.h"

#define NUM_PXT 139
extern PIXTONEPARAMETER gPtpTable[NUM_PXT];

BOOL LoadSurfaces();
void MakeGenericSurfaces();
void LoadSounds();
