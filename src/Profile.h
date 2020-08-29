#pragma once

#include <string>

#include "WindowsWrapper.h"

#include "ArmsItem.h"
#include "SelStage.h"
#include "Stage.h"
#include "Game.h"

typedef struct PROFILE
{
	BOOL flag;
	char code[8];
	int stage;
	MusicID music;
	GameDifficulty difficulty;
	int x;
	int y;
	int direct;
	short max_life;
	short star;
	short life;
	short a;
	int select_arms;
	int select_item;
	int equip;
	int unit;
	int counter;
	ARMS arms[8];
	ITEM items[32];
	PERMIT_STAGE permitstage[8];
	signed char permit_mapping[0x80];
	char FLAG[4];
	unsigned char flags[1000];
	char time[100];
} PROFILE;

extern const char* const gDefaultName;
extern const char* const gDefaultExt;
extern const char* const gProfileCode;
extern unsigned int gProfileId;

BOOL GetProfile(unsigned int id, PROFILE *profile);
BOOL SaveProfile(unsigned int id);
BOOL LoadProfile(unsigned int id);
BOOL InitializeGame(unsigned int id);
BOOL DeleteProfile(unsigned int id);
