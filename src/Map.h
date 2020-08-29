#pragma once
#include <stddef.h>
#include "WindowsWrapper.h"
#include <string>

typedef struct MAP_DATA
{
	unsigned char *data = NULL;
	unsigned char atrb[0x100];
	short width;
	short length;
} MAP_DATA;

extern MAP_DATA gMap;

BOOL LoadMapData2(std::string path_map);
BOOL LoadAttributeData(std::string path_atrb);
void EndMapData(void);
void ReleasePartsImage(void);
void GetMapData(unsigned char **data, short *mw, short *ml);
unsigned char GetAttribute(int x, int y);
void DeleteMapParts(int x, int y);
void ShiftMapParts(int x, int y);
BOOL ChangeMapParts(int x, int y, unsigned char no);
void PutStage_Back(int fx, int fy);
void PutStage_Front(int fx, int fy);
void PutMapDataVector(int fx, int fy);
