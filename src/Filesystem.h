#pragma once
#include <string>
#include "WindowsWrapper.h"

enum FilesystemSource
{
	FSS_Module,
	FSS_Base,
	FSS_Mod,
};

BOOL Filesystem_Init(std::string modulePath);
void Filesystem_SetMod(std::string mod);
std::string Filesystem_GetMod();

bool FileExists(std::string path);

std::string FindFile(FilesystemSource source, std::string name);
FILE *OpenFile(FilesystemSource source, std::string name, const char *type);
