#include "File.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>

unsigned char* LoadFileToMemory(std::string file_path, size_t *file_size)
{
	unsigned char *buffer = NULL;

	FILE *file = fopen(file_path.c_str(), "rb");

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

#define SIGNIFICANDBITS (52)
#define EXPBITS (11)

double File_ReadDouble(FILE *fp)
{
	//Read data
	unsigned char buff[8];
	for (int i = 0; i < 8; i++)
		buff[i] = fgetc(fp);
	
	//Get sign and exponent
	int sign = buff[0] & 0x80 ? -1 : 1;
	int exponent = ((buff[0] & 0x7F) << 4) | ((buff[1] & 0xF0) >> 4);
	
	//Read in the mantissa. Top bit is 0.5, the successive bits half
	double bitval = 0.5;
	int maski = 1;
	int mask = 0x08;
	double fnorm = 0.0;
	
	for (int i = 0; i < SIGNIFICANDBITS; i++)
	{
		if (buff[maski] & mask)
			fnorm += bitval;
		bitval /= 2.0;
		mask >>= 1;
		if (mask == 0)
		{
			mask = 0x80;
			maski++;
		}
	}
	
	//Case for 0
	if (exponent == 0 && fnorm == 0)
		return 0.0;
	
	//Get shift
	int expbits = 11;
	int shift = exponent - ((1 << (EXPBITS - 1)) - 1); //Exponent = shift + bias
	
	//NaNs has exp 1024 and non-zero mantissa
	if (shift == 1024 && fnorm != 0)
		return sqrt(-1.0);
	
	//Infinity
	if (shift == 1024 && fnorm == 0)
	{
#ifdef INFINITY
		return sign == 1 ? INFINITY : -INFINITY;
#else
		return  (sign * 1.0) / 0.0;
#endif
	}
	
	if (shift > -1023)
	{
		double answer = ldexp(fnorm + 1.0, shift);
		return answer * sign;
	}
	else
	{
		//Denormalised numbers
		if (fnorm == 0.0)
			return 0.0;
		shift = -1022;
		while (fnorm < 1.0)
		{
			fnorm *= 2;
			shift--;
		}
		
		double answer = ldexp(fnorm, shift);
		return answer * sign;
	}
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
