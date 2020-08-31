#pragma once

#include "WindowsWrapper.h"

#define NPC_FLAG_NUM	8000
#define SKIP_FLAG_NUM	64

extern unsigned char gFlagNPC[(NPC_FLAG_NUM + 7) / 8];
extern unsigned char gSkipFlag[(SKIP_FLAG_NUM+7) / 8];

void InitFlags(void);
void InitSkipFlags(void);
void SetNPCFlag(long a);
void CutNPCFlag(long a);
BOOL GetNPCFlag(long a);
void SetSkipFlag(long a);
void CutSkipFlag(long a);
BOOL GetSkipFlag(long a);
