#include "Filesystem.h"
#include "Main.h"
#include <sys/stat.h>

//Filesystem interface
struct Filesystem
{
	std::string module_path, base_path, season_path, mod_path; 
} fs = {};

BOOL InitFilesystem(std::string modulePath)
{
	//Initialize paths
	fs.module_path = modulePath + '/';
	fs.base_path = modulePath + "/data/base/";
	switch (g_GameSeason)
	{
		case GS_Halloween:
			fs.season_path = modulePath + "/data/Halloween/season/";
			break;
		case GS_Christmas:
			fs.season_path = modulePath + "/data/Christmas/season/";
			break;
		default:
			fs.season_path = "";
			break;
	}
	fs.mod_path = "";
	return TRUE;
}

bool FileExists(std::string path)
{
	struct stat s;
	if(stat(path.c_str(), &s) == 0)
		return true;
	return false;
}

std::string FindFile(FilesystemSource source, std::string name)
{
	//Get path to search
	std::string path;
	switch (source)
	{
		case FSS_Module:
			path = fs.module_path + name;
			break;
		case FSS_Base:
			path = fs.base_path + name;
			break;
		case FSS_Mod:
			if (!fs.mod_path.empty() && FileExists(path = (fs.mod_path + name)))
				break;
			if (!fs.season_path.empty() && FileExists(path = (fs.season_path + name)))
				break;
			path = fs.base_path + name;
			break;
	}
	
	return path;
}

FILE *OpenFile(FilesystemSource source, std::string name, const char *type)
{
	return fopen(FindFile(source, name).c_str(), type);
}
