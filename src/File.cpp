#include "File.h"

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
	#define CASE_INSENSITIVE
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#ifdef CASE_INSENSITIVE
	#include <dirent.h>
#endif

std::string FindFilePath(const char *path)
{
	#ifdef CASE_INSENSITIVE
		//Get path
		std::string past = std::string(path);
		size_t ls = past.find_last_of("/\\");
		if (ls != std::string::npos)
			past = past.substr(0, ls + 1);
		else
			past = "./";
		
		std::string low = std::string(path);
		std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c){return std::tolower(c);});
		
		//Open directory
		DIR *dir = opendir(past.c_str());
		dirent *ent;
		
		if (dir != nullptr)
		{
			while ((ent = readdir (dir)) != NULL)
			{
				if (ent->d_type == DT_REG)
				{
					std::string this_path = std::string(past) + ent->d_name;
					std::string low_path = this_path;
					std::transform(low_path.begin(), low_path.end(), low_path.begin(), [](unsigned char c){return std::tolower(c);});
					if (low_path == low)
						return this_path;
				}
			}
		}
	#endif
	return std::string(path);
}

FILE *FindFile(const char *path, const char *type)
{
	return fopen(FindFilePath(path).c_str(), type);
}

unsigned char* LoadFileToMemory(const char *file_path, size_t *file_size)
{
	unsigned char *buffer = NULL;

	FILE *file = FindFile(file_path, "rb");

	if (file != NULL)
	{
		if (!fseek(file, 0, SEEK_END))
		{
			const long _file_size = ftell(file);

			if (_file_size >= 0)
			{
				rewind(file);
				buffer = (unsigned char*)malloc(_file_size);

				if (buffer != NULL)
				{
					if (fread(buffer, _file_size, 1, file) == 1)
					{
						fclose(file);
						*file_size = (size_t)_file_size;
						return buffer;
					}

					free(buffer);
				}
			}
		}

		fclose(file);
	}

	return NULL;
}

unsigned short File_ReadBE16(FILE *stream)
{
	unsigned char bytes[2];

	fread(bytes, 2, 1, stream);

	return (bytes[0] << 8) | bytes[1];
}

unsigned long File_ReadBE32(FILE *stream)
{
	unsigned char bytes[4];

	fread(bytes, 4, 1, stream);

	return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

unsigned short File_ReadLE16(FILE *stream)
{
	unsigned char bytes[2];

	fread(bytes, 2, 1, stream);

	return (bytes[1] << 8) | bytes[0];
}

unsigned long File_ReadLE32(FILE *stream)
{
	unsigned char bytes[4];

	fread(bytes, 4, 1, stream);

	return (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
}

void File_WriteBE16(unsigned short value, FILE *stream)
{
	for (unsigned int i = 2; i-- != 0;)
		fputc(value >> (8 * i), stream);
}

void File_WriteBE32(unsigned long value, FILE *stream)
{
	for (unsigned int i = 4; i-- != 0;)
		fputc(value >> (8 * i), stream);
}

void File_WriteLE16(unsigned short value, FILE *stream)
{
	for (unsigned int i = 0; i < 2; ++i)
		fputc(value >> (8 * i), stream);
}

void File_WriteLE32(unsigned long value, FILE *stream)
{
	for (unsigned int i = 0; i < 4; ++i)
		fputc(value >> (8 * i), stream);
}
