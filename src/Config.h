#pragma once
#include "WindowsWrapper.h"
#include "Music.h"

struct CONFIG
{
	//Proof
	char proof[16]{};
	
	//Sound settings
	MusicType music_type = MT_Ogg11;
	
	//Video settings
	bool original_graphics = false;
};

extern const char* const gConfigName;
extern const char* const gProof;

extern CONFIG gConfig;

void LoadConfigData();
void SaveConfigData();
