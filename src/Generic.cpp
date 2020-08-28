#include "Generic.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "WindowsWrapper.h"

#include "Main.h"
#include "File.h"
#include "Filesystem.h"

void GetCompileDate(int *year, int *month, int *day)
{
	int i;
	char strMonth[0x10];

	const char *table[13] = {
		"XXX",
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
	};

	sscanf(__DATE__, "%s %d %d", strMonth, day, year);	// The expansion of __DATE__ is not reproductible. TODO : Think about changing this to be reproductible

	for (i = 0; i < 12; ++i)	// This being 12 instead of 13 might be a bug, but it works anyway by accident
		if (!memcmp(&strMonth, table[i], 3))
			break;

	*month = i;
}

BOOL GetCompileVersion(int *v1, int *v2, int *v3, int *v4)
{
	*v1 = 1;
	*v2 = 0;
	*v3 = 0;
	*v4 = 6;
	return TRUE;
}

void DeleteLog(void)
{
	remove(FindFile(FSS_Module, "debug.txt").c_str());
}

BOOL WriteLog(const char *string, int value1, int value2, int value3)
{
	FILE *fp = OpenFile(FSS_Module, "debug.txt", "a+");
	if (fp == NULL)
		return FALSE;

	fprintf(fp, "%s,%d,%d,%d\n", string, value1, value2, value3);
	fclose(fp);
	return TRUE;
}

BOOL IsKeyFile(const char *name)
{
	FILE *file = OpenFile(FSS_Module, name, "rb");
	if (file == NULL)
		return FALSE;

	fclose(file);
	return TRUE;
}

long GetFileSizeLong(const char *path)
{
	long len;
	FILE *fp = fopen(path, "rb");
	if (fp == NULL)
		return -1;

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fclose(fp);
	return len;
}

BOOL ErrorLog(const char *string, int value)
{
	std::string path = FindFile(FSS_Module, "error.log");
	if (GetFileSizeLong(path.c_str()) > 0x19000)	// Purge the error log if it gets too big, I guess
		remove(path.c_str());

	FILE *fp = fopen(path.c_str(), "a+");
	if (fp == NULL)
		return FALSE;

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

BOOL IsEnableBitmap(const char *path)
{
	return TRUE;
}
