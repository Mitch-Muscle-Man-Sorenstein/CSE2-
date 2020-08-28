#pragma once
#include <string>
#include "WindowsWrapper.h"

enum FilesystemSource
{
	FSS_Module,
	FSS_Base,
	FSS_Mod,
};

BOOL InitFilesystem(std::string modulePath);
std::string FindFile(FilesystemSource source, std::string name);
FILE *OpenFile(FilesystemSource source, std::string name, const char *type);
