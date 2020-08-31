#pragma once
#include "WindowsWrapper.h"
#include <string>

void DebugLog();
BOOL WriteLog(const char *string, int value1, int value2, int value3);
BOOL ErrorLog(const char *string, int value);
BOOL IsShiftJIS(unsigned char c);
