#include "Generic.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "WindowsWrapper.h"

#include "Main.h"
#include "File.h"
#include "Filesystem.h"

BOOL WriteLog(const char *string, int value1, int value2, int value3)
{
	//Open log file
	FILE *fp = OpenFile(FSS_Module, "debug.txt", "a+");
	if (fp == NULL)
		return FALSE;
	
	//Write to log file
	fprintf(fp, "%s,%d,%d,%d\n", string, value1, value2, value3);
	fclose(fp);
	return TRUE;
}

BOOL ErrorLog(const char *string, int value)
{
	//Open log file
	std::string path = FindFile(FSS_Module, "error.log");
	FILE *fp = fopen(path.c_str(), "a+");
	if (fp == NULL)
		return FALSE;
	
	//Write error to file
	fprintf(fp, "%s,%d\n", string, value);
	fclose(fp);
	return TRUE;
}

BOOL IsShiftJIS(unsigned char c)
{
	if (c >= 0x81 && c <= 0x9F)
		return TRUE;
	if (c >= 0xE0 && c <= 0xEF)
		return TRUE;
	return FALSE;
}
