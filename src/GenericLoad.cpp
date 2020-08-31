#include "GenericLoad.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdint.h>
#include <iostream>

#include "WindowsWrapper.h"

#include "CommonDefines.h"
#include "Draw.h"
#include "Ending.h"
#include "PixTone.h"
#include "Sound.h"
#include "Main.h"
#include "Filesystem.h"
#include "File.h"

#include "Sound02.h"

PIXTONEPARAMETER *gPtpTable = nullptr;
size_t gPtpTable_size;

//PixTone loading
BOOL LoadPixToneTable()
{
	//Open file
	FILE *fp = OpenFile(FSS_Mod, "pixtone.tbl", "rb");
	if (fp == NULL)
		return FALSE;
	
	//Get size
	fseek(fp, 0, SEEK_END);
	size_t entries = ftell(fp) / 0x60;
	fseek(fp, 0, SEEK_SET);
	
	//Allocate new ptp table
	PIXTONEPARAMETER *new_tbl = new PIXTONEPARAMETER[entries];
	if (new_tbl == NULL)
	{
		fclose(fp);
		return FALSE;
	}
	
	//Read contents
	for (size_t i = 0; i < entries; i++)
	{
		//Read ptp data
		PIXTONEPARAMETER ptp;
		
		ptp.use = File_ReadLE32(fp);
		ptp.size = File_ReadLE32(fp);
		
		ptp.oMain.model = File_ReadLE32(fp);
		ptp.oMain.num = File_ReadDouble(fp);
		ptp.oMain.top = File_ReadLE32(fp);
		ptp.oMain.offset = File_ReadLE32(fp);
		
		ptp.oPitch.model = File_ReadLE32(fp);
		ptp.oPitch.num = File_ReadDouble(fp);
		ptp.oPitch.top = File_ReadLE32(fp);
		ptp.oPitch.offset = File_ReadLE32(fp);
		
		ptp.oVolume.model = File_ReadLE32(fp);
		ptp.oVolume.num = File_ReadDouble(fp);
		ptp.oVolume.top = File_ReadLE32(fp);
		ptp.oVolume.offset = File_ReadLE32(fp);
		
		ptp.initial = File_ReadLE32(fp);
		ptp.pointAx = File_ReadLE32(fp);
		ptp.pointAy = File_ReadLE32(fp);
		ptp.pointBx = File_ReadLE32(fp);
		ptp.pointBy = File_ReadLE32(fp);
		ptp.pointCx = File_ReadLE32(fp);
		ptp.pointCy = File_ReadLE32(fp);
		
		//Determine if this is different
		if (gPtpTable != NULL && i < gPtpTable_size)
		{
			PIXTONEPARAMETER *optp = &gPtpTable[i];
			if ((ptp.use ==				optp->use) &&
				(ptp.size ==			optp->size) &&
				(ptp.oMain.model ==		optp->oMain.model) &&
				(ptp.oMain.num ==		optp->oMain.num) &&
				(ptp.oMain.top ==		optp->oMain.top) &&
				(ptp.oMain.offset ==	optp->oMain.offset) &&
				(ptp.oPitch.model ==	optp->oPitch.model) &&
				(ptp.oPitch.num ==		optp->oPitch.num) &&
				(ptp.oPitch.top ==		optp->oPitch.top) &&
				(ptp.oPitch.offset ==	optp->oPitch.offset) &&
				(ptp.oVolume.model ==	optp->oVolume.model) &&
				(ptp.oVolume.num ==		optp->oVolume.num) &&
				(ptp.oVolume.top ==		optp->oVolume.top) &&
				(ptp.oVolume.offset ==	optp->oVolume.offset) &&
				(ptp.initial ==			optp->initial) &&
				(ptp.pointAx ==			optp->pointAx) &&
				(ptp.pointAy ==			optp->pointAy) &&
				(ptp.pointBx ==			optp->pointBx) &&
				(ptp.pointBy ==			optp->pointBy) &&
				(ptp.pointCx ==			optp->pointCx) &&
				(ptp.pointCy ==			optp->pointCy))
				ptp.mod = FALSE;
			else
				ptp.mod = TRUE;
		}
		else
		{
			ptp.mod = TRUE;
		}
		
		//Copy to table
		new_tbl[i] = ptp;
	}
	
	//Replace previous ptp table and close file
	delete[] gPtpTable;
	gPtpTable = new_tbl;
	gPtpTable_size = entries;
	fclose(fp);
	return TRUE;
}

void ReleasePixToneTable()
{
	delete[] gPtpTable;
}

BOOL LoadPixToneObject(int ptp, int ptp_num, int no)
{
	if (no < 0 || no >= SE_MAX || ptp < 0 || (ptp + ptp_num) > gPtpTable_size)
		return FALSE;
	
	for (int i = ptp; i < ptp + ptp_num; i++)
	{
		if (gPtpTable[i].mod)
		{
			if (!MakePixToneObject(&gPtpTable[ptp], ptp_num, no))
				return FALSE;
		}
	}
	return TRUE;
}

//Asset loading
BOOL LoadSurfaces(void)
{
	if (!MakeSurface_File("Nicalis", SURFACE_ID_NICALIS))
		return FALSE;
	if (!MakeSurface_File("MyChar", SURFACE_ID_MY_CHAR))
		return FALSE;
	if (!MakeSurface_File("Title", SURFACE_ID_TITLE))
		return FALSE;
	if (!MakeSurface_File("ArmsImage", SURFACE_ID_ARMS_IMAGE))
		return FALSE;
	if (!MakeSurface_File("UI", SURFACE_ID_UI))
		return FALSE;
	if (!MakeSurface_File("Arms", SURFACE_ID_ARMS))
		return FALSE;
	if (!MakeSurface_File("ItemImage", SURFACE_ID_ITEM_IMAGE))
		return FALSE;
	if (!MakeSurface_File("StageImage", SURFACE_ID_STAGE_ITEM))
		return FALSE;
	if (!MakeSurface_File("Npc/NpcSym", SURFACE_ID_NPC_SYM))
		return FALSE;
	if (!MakeSurface_File("Npc/NpcRegu", SURFACE_ID_NPC_REGU))
		return FALSE;
	if (!MakeSurface_File("TextBox", SURFACE_ID_TEXT_BOX))
		return FALSE;
	if (!MakeSurface_File("Caret", SURFACE_ID_CARET))
		return FALSE;
	if (!MakeSurface_File("Bullet", SURFACE_ID_BULLET))
		return FALSE;
	if (!MakeSurface_File("Face", SURFACE_ID_FACE))
		return FALSE;
	if (!MakeSurface_File("Fade", SURFACE_ID_FADE))
		return FALSE;
	if (!MakeSurface_Resource("CREDIT01", SURFACE_ID_CREDITS_IMAGE))
		return FALSE;
	return TRUE;
}

void MakeGenericSurfaces()
{
	MakeSurface_Generic(WINDOW_WIDTH, WINDOW_HEIGHT, SURFACE_ID_SCREEN_GRAB, TRUE);
	MakeSurface_Generic(WINDOW_WIDTH, WINDOW_HEIGHT, SURFACE_ID_LEVEL_BACKGROUND, FALSE);
	MakeSurface_Generic(WINDOW_WIDTH, WINDOW_HEIGHT, SURFACE_ID_MAP, TRUE);
	MakeSurface_Generic(320, 240, SURFACE_ID_CASTS, FALSE);
	MakeSurface_Generic(256, 256, SURFACE_ID_LEVEL_TILESET, FALSE);
	MakeSurface_Generic(40, 240, SURFACE_ID_VALUE_VIEW, FALSE);
	MakeSurface_Generic(320, 240, SURFACE_ID_LEVEL_SPRITESET_1, FALSE);
	MakeSurface_Generic(320, 240, SURFACE_ID_LEVEL_SPRITESET_2, FALSE);
}

BOOL LoadSounds()
{
	if (!LoadPixToneTable())
		return FALSE;
	
	if (LoadPixToneObject(0, 2, 32) &&
		LoadPixToneObject(2, 2, 33) &&
		LoadPixToneObject(4, 2, 34) &&
		LoadPixToneObject(6, 1, 15) &&
		LoadPixToneObject(7, 1, 24) &&
		LoadPixToneObject(8, 1, 23) &&
		LoadPixToneObject(9, 2, 50) &&
		LoadPixToneObject(11, 2, 51) &&
		LoadPixToneObject(33, 1, 1) &&
		LoadPixToneObject(56, 1, 29) &&
		LoadPixToneObject(61, 1, 43) &&
		LoadPixToneObject(62, 3, 44) &&
		LoadPixToneObject(65, 1, 45) &&
		LoadPixToneObject(66, 1, 46) &&
		LoadPixToneObject(68, 1, 47) &&
		LoadPixToneObject(49, 3, 35) &&
		LoadPixToneObject(52, 3, 39) &&
		LoadPixToneObject(13, 2, 52) &&
		LoadPixToneObject(28, 2, 53) &&
		LoadPixToneObject(15, 2, 70) &&
		LoadPixToneObject(17, 2, 71) &&
		LoadPixToneObject(19, 2, 72) &&
		LoadPixToneObject(30, 1, 5) &&
		LoadPixToneObject(32, 1, 11) &&
		LoadPixToneObject(35, 1, 4) &&
		LoadPixToneObject(46, 2, 25) &&
		LoadPixToneObject(48, 1, 27) &&
		LoadPixToneObject(54, 2, 28) &&
		LoadPixToneObject(39, 1, 14) &&
		LoadPixToneObject(23, 2, 16) &&
		LoadPixToneObject(25, 3, 17) &&
		LoadPixToneObject(34, 1, 18) &&
		LoadPixToneObject(36, 2, 20) &&
		LoadPixToneObject(31, 1, 22) &&
		LoadPixToneObject(41, 2, 26) &&
		LoadPixToneObject(43, 1, 21) &&
		LoadPixToneObject(44, 2, 12) &&
		LoadPixToneObject(57, 2, 38) &&
		LoadPixToneObject(59, 1, 31) &&
		LoadPixToneObject(60, 1, 42) &&
		LoadPixToneObject(69, 1, 48) &&
		LoadPixToneObject(70, 2, 49) &&
		LoadPixToneObject(72, 1, 100) &&
		LoadPixToneObject(73, 3, 101) &&
		LoadPixToneObject(76, 2, 54) &&
		LoadPixToneObject(78, 2, 102) &&
		LoadPixToneObject(80, 2, 103) &&
		LoadPixToneObject(81, 1, 104) &&
		LoadPixToneObject(82, 1, 105) &&
		LoadPixToneObject(83, 2, 106) &&
		LoadPixToneObject(85, 1, 107) &&
		LoadPixToneObject(86, 1, 30) &&
		LoadPixToneObject(87, 1, 108) &&
		LoadPixToneObject(88, 1, 109) &&
		LoadPixToneObject(89, 1, 110) &&
		LoadPixToneObject(90, 1, 111) &&
		LoadPixToneObject(91, 1, 112) &&
		LoadPixToneObject(92, 1, 113) &&
		LoadPixToneObject(93, 2, 114) &&
		LoadPixToneObject(95, 2, 150) &&
		LoadPixToneObject(97, 2, 151) &&
		LoadPixToneObject(99, 1, 152) &&
		LoadPixToneObject(100, 1, 153) &&
		LoadPixToneObject(101, 2, 154) &&
		LoadPixToneObject(111, 2, 155) &&
		LoadPixToneObject(95, 2, 156) &&
		LoadPixToneObject(97, 2, 157) &&
		LoadPixToneObject(103, 2, 56) &&
		LoadPixToneObject(105, 2, 40) &&
		LoadPixToneObject(105, 2, 41) &&
		LoadPixToneObject(107, 2, 37) &&
		LoadPixToneObject(109, 2, 57) &&
		LoadPixToneObject(113, 3, 115) &&
		LoadPixToneObject(116, 1, 104) &&
		LoadPixToneObject(117, 3, 116) &&
		LoadPixToneObject(120, 2, 58) &&
		LoadPixToneObject(122, 2, 55) &&
		LoadPixToneObject(124, 2, 117) &&
		LoadPixToneObject(126, 1, 59) &&
		LoadPixToneObject(127, 1, 60) &&
		LoadPixToneObject(128, 1, 61) &&
		LoadPixToneObject(129, 2, 62) &&
		LoadPixToneObject(131, 2, 63) &&
		LoadPixToneObject(133, 2, 64) &&
		LoadPixToneObject(135, 1, 65) &&
		LoadPixToneObject(136, 1, 3) &&
		LoadPixToneObject(137, 1, 6))
	{
		if (lpSECONDARYBUFFER[2] == NULL)
		{
			if ((lpSECONDARYBUFFER[2] = AudioBackend_CreateSound(22050, sound02_data, sound02_size)) != NULL)
				return TRUE;
		}
	}
	return FALSE;
}
