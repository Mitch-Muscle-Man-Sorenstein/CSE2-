#pragma once

#include "WindowsWrapper.h"

void DebugLog(void);
BOOL WriteLog(const char *string, int value1, int value2, int value3);
BOOL IsKeyFile(const char *name);
long GetFileSizeLong(const char *path);
BOOL ErrorLog(const char *string, int value);
BOOL IsShiftJIS(unsigned char c);
