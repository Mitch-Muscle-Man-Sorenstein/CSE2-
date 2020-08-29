#include "Stage.h"

#include <string>

#include "WindowsWrapper.h"

#include "Main.h"
#include "File.h"
#include "Back.h"
#include "Boss.h"
#include "Bullet.h"
#include "Caret.h"
#include "Draw.h"
#include "Flash.h"
#include "Frame.h"
#include "Map.h"
#include "MapName.h"
#include "MyChar.h"
#include "NpChar.h"
#include "Music.h"
#include "TextScr.h"
#include "ValueView.h"
#include "Filesystem.h"

#ifdef JAPANESE
#define STAGE_ENTRY(parts, map, bkType, back, npc, boss, boss_no, name_en, name_jp) {parts, map, bkType, back, npc, boss, boss_no, name_jp}
#else
#define STAGE_ENTRY(parts, map, bkType, back, npc, boss, boss_no, name_en, name_jp) {parts, map, bkType, back, npc, boss, boss_no, name_en}
#endif

int gStageNo;
MusicID gMusicNo;
unsigned int gOldPos;
MusicID gOldNo;

STAGE_TABLE *gTMT = nullptr;
size_t gTMT_size;

BOOL LoadStageTable()
{
	FILE *fp = OpenFile(FSS_Mod, "stage.tbl", "rb");
	if (!fp)
		return FALSE;
	
	fseek(fp, 0, SEEK_END);
	gTMT_size = ftell(fp) / 0xE5;
	fseek(fp, 0, SEEK_SET);
	
	ReleaseStageTable();
	if ((gTMT = new STAGE_TABLE[gTMT_size]) == NULL)
		return FALSE;
	
	STAGE_TABLE *gTMTp = gTMT;
	for (size_t i = 0; i < gTMT_size; i++)
	{
		fread(gTMTp->parts, 0x20, 1, fp);
		fread(gTMTp->map, 0x20, 1, fp);
		gTMTp->bkType = File_ReadLE32(fp);
		fread(gTMTp->back, 0x20, 1, fp);
		fread(gTMTp->npc, 0x20, 1, fp);
		fread(gTMTp->boss, 0x20, 1, fp);
		gTMTp->boss_no = fgetc(fp);
		fread(gTMTp->name_jp, 0x20, 1, fp);
		fread(gTMTp->name, 0x20, 1, fp);
		gTMTp++;
	}
	
	fclose(fp);
	return TRUE;
}

void ReleaseStageTable()
{
	if (gTMT != NULL)
	{
		delete[] gTMT;
		gTMT = NULL;
	}
}

BOOL TransferStage(int no, int w, int x, int y)
{
	std::string path;
	std::string path_dir;
	
	//If we can't load from the stage table, fail immediately
	if (gTMT == NULL || no > gTMT_size)
		return FALSE;
	
	//Move character
	SetMyCharPosition(x * 0x10 * 0x200, y * 0x10 * 0x200);
	
	//Get path
	path_dir = "Stage";
	
	//Load tileset
	path = path_dir + "/Prt" + gTMT[no].parts;
	if (!ReloadBitmap_File(path, SURFACE_ID_LEVEL_TILESET))
		return FALSE;
	
	path = path_dir + '/' + gTMT[no].parts + ".pxa";
	if (!LoadAttributeData(path))
		return FALSE;
	
	//Load tilemap
	path = path_dir + '/' + gTMT[no].map + ".pxm";
	if (!LoadMapData2(path))
		return FALSE;
	
	//Load NPCs
	path = path_dir + '/' + gTMT[no].map + ".pxe";
	if (!LoadEvent(path))
		return FALSE;
	
	//Load script
	path = path_dir + '/' + gTMT[no].map + ".tsc";
	if (!LoadTextScript_Stage(path))
		return FALSE;
	
	//Load background
	path = gTMT[no].back;
	if (!InitBack(path, gTMT[no].bkType))
		return FALSE;
	
	//Get path
	path_dir = "Npc";
	
	//Load NPC sprite sheets
	path = path_dir + "/Npc" + gTMT[no].npc;
	if (!ReloadBitmap_File(path, SURFACE_ID_LEVEL_SPRITESET_1))
		return FALSE;
	
	path = path_dir + "/Npc" + gTMT[no].boss;
	if (!ReloadBitmap_File(path, SURFACE_ID_LEVEL_SPRITESET_2))
		return FALSE;
	
	//Load map name
	ReadyMapName(gTMT[no].name);
	
	StartTextScript(w);
	SetFrameMyChar();
	ClearBullet();
	InitCaret();
	ClearValueView();
	ResetQuake();
	InitBossChar(gTMT[no].boss_no);
	ResetFlash();
	gStageNo = no;
	return TRUE;
}

// Music
char **gMusicTable = nullptr;
size_t gMusicTable_size;

BOOL LoadMusicTable()
{
	FILE *fp = OpenFile(FSS_Mod, "music.tbl", "rb");
	if (!fp)
		return FALSE;
	
	fseek(fp, 0, SEEK_END);
	gMusicTable_size = ftell(fp) / 0x10;
	fseek(fp, 0, SEEK_SET);
	
	ReleaseMusicTable();
	if ((gMusicTable = new char*[gMusicTable_size]) == NULL)
		return FALSE;
	
	char **musicp = gMusicTable;
	for (size_t i = 0; i < gMusicTable_size; i++)
	{
		*musicp = new char[0x10];
		fread(*musicp, 0x10, 1, fp);
		musicp++;
	}
	
	fclose(fp);
	return TRUE;
}

void ReleaseMusicTable()
{
	if (gMusicTable != NULL)
	{
		for (size_t i = 0; i < gMusicTable_size; i++)
			delete[] gMusicTable[i];
		delete[] gMusicTable;
	}
}

void ChangeMusic(MusicID no)
{
	if (no != MUS_SILENCE && no == gMusicNo)
		return;
	
	// Stop and keep track of old song
	gOldPos = GetMusicPosition();
	gOldNo = gMusicNo;
	StopMusic();

	// Load music
	if (gMusicTable != NULL && no < gMusicTable_size)
		LoadMusic(gMusicTable[no]);

	// Reset position, volume, and then play the song
	ChangeMusicVolume(100);
	SetMusicPosition(0);
	PlayMusic();
	gMusicNo = no;
}

void ReCallMusic(void)
{
	// Stop old music
	StopMusic();

	// Load music that was playing before
	LoadMusic(gMusicTable[gOldNo]);

	// Reset position, volume, and then play the song
	SetMusicPosition(gOldPos);
	ChangeMusicVolume(100);
	PlayMusic();
	gMusicNo = gOldNo;
}
