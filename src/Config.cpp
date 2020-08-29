#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "WindowsWrapper.h"

#include "Config.h"
#include "File.h"
#include "Filesystem.h"

const char* const gConfigName = "Config.dat";
const char* const gProof = "DOUKUTSU20200829";

CONFIG gConfig{};

void LoadConfigData()
{
	//Initialize configuration data
	gConfig = CONFIG{};
	
	//Open file
	FILE *fp = OpenFile(FSS_Module, gConfigName, "rb");
	if (fp == NULL)
		return;
	
	//Read and verify proof
	fread(gConfig.proof, 16, 1, fp);
	if (strncmp(gConfig.proof, gProof, 16))
	{
		fclose(fp);
		return;
	}
	
	//Read audio settings
	char music_type = fgetc(fp);
	if (music_type >= 0 && music_type < MT_MAX)
		gConfig.music_type = (MusicType)music_type;
	
	//Read video settings
	gConfig.original_graphics = (bool)fgetc(fp);
	
	//Close file
	fclose(fp);
	return;
}

void SaveConfigData()
{
	//Open file
	FILE *fp = OpenFile(FSS_Module, gConfigName, "wb");
	if (fp == NULL)
		return;
	
	//Write proof
	fwrite(gProof, 16, 1, fp);
	
	//Write audio settings
	fputc((char)gConfig.music_type, fp);
	
	//Write video settings
	fputc((char)gConfig.original_graphics, fp);
	
	//Close file
	fclose(fp);
	return;
}
