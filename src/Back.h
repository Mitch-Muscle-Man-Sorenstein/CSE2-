#pragma once
#include "WindowsWrapper.h"
#include <string>

typedef struct BACK
{
	int partsW;
	int partsH;
	int numX;
	int numY;
	int type;
	int fx;
} BACK;

extern BACK gBack;
extern int gWaterY;

BOOL InitBack(std::string fName, int type);
void ActBack(void);
void PutBack(int fx, int fy);
void PutFront(int fx, int fy);
