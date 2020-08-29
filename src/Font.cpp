#include "Font.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

#include "WindowsWrapper.h"

#include "Draw.h"
#include "File.h"
#include "Bitmap.h"

#define BITMAP_BPP 3

//Bitmap font structures
struct BMF_Info
{
	int16_t fontSize; //Yeah, according to the documentation this is signed
	uint8_t bitField; //bit 0: smooth, bit 1: unicode, bit 2: italic, bit 3: bold, bit 4: fixedHeigth, bits 5-7: reserved
	uint8_t charSet;
	uint16_t stretchH;
	uint8_t aa;
	uint8_t paddingUp;
	uint8_t paddingRight;
	uint8_t paddingDown;
	uint8_t paddingLeft;
	uint8_t spacingHoriz;
	uint8_t spacingVert;
	uint8_t outline;
	char *fontName;
};

struct BMF_Common
{
	uint16_t lineHeight;
	uint16_t base;
	uint16_t scaleW;
	uint16_t scaleH;
	uint16_t pages;
	uint8_t bitField; //bits 0-6: reserved, bit 7: packed
	uint8_t alphaChnl;
	uint8_t redChnl;
	uint8_t greenChnl;
	uint8_t blueChnl;
};

struct BMF_Pages
{
	char **pageNames;
};

struct BMF_Char
{
	uint32_t id;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	int16_t xoffset;
	int16_t yoffset;
	int16_t xadvance;
	uint8_t page;
	uint8_t chnl;
};

struct BMF_Chars
{
	size_t numChars;
	BMF_Char *chars;
};

struct BMF_KerningPair
{
	uint32_t first;
	uint32_t second;
	int16_t amount;
};

struct BMF_KerningPairs
{
	size_t numKerningPairs;
	BMF_KerningPair *kerningPairs;
};

//Font object structure (contains all the BMF data)
struct FontObject
{
	//BMF data
	BMF_Info *blockInfo;
	BMF_Common *blockCommon;
	BMF_Pages *blockPages;
	BMF_Chars *blockChars;
	BMF_KerningPairs *blockKerningPairs;
	
	//Font data
	std::string pagePath;
	RenderBackend_Surface **pages;
};

//Font loading (from data is unsupported)
FontObject* LoadFontFromData(const unsigned char *data, size_t data_size, unsigned int cell_width, unsigned int cell_height)
{
	//Unsupported
	return NULL;
}

FontObject* LoadFont(std::string font_filename, unsigned int cell_width, unsigned int cell_height)
{
	(void)cell_width;
	(void)cell_height;
	
	FontObject *font_object = (FontObject*)malloc(sizeof(FontObject));
	
	if (font_object != NULL)
	{
		//Clear font object data
		memset(font_object, 0, sizeof(FontObject));
		
		//Get the path that holds the .fnt file
		std::string pagePath = font_filename;
		size_t last_pos = pagePath.find_last_of("/\\");
		if (last_pos != std::string::npos)
			font_object->pagePath = pagePath.substr(0, last_pos + 1);
		else
			font_object->pagePath = "";
		
		//Open file
		FILE *fp = fopen(font_filename.c_str(), "rb");
			
		if (fp != NULL)
		{
			//Get size
			fseek(fp, 0, SEEK_END);
			size_t size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			
			//Check header (BMF\x03)
			bool error = false;
			char header[4];
			fread(header, 1, 4, fp);
			
			if (!memcmp(header, "BMF\x03", 4))
			{
				//Read all blocks of the file
				while (ftell(fp) < size && error == false)
				{
					uint8_t type = fgetc(fp);
					uint32_t size = File_ReadLE32(fp);
					
					switch (type)
					{
						case 1: //Info
						{
							if ((font_object->blockInfo = (BMF_Info*)malloc(sizeof(BMF_Info))) == NULL)
							{
								error = true;
								break;
							}
							font_object->blockInfo->fontSize = File_ReadLE16(fp);
							font_object->blockInfo->bitField = fgetc(fp);
							font_object->blockInfo->charSet = fgetc(fp);
							font_object->blockInfo->stretchH = File_ReadLE16(fp);
							font_object->blockInfo->aa = fgetc(fp);
							font_object->blockInfo->paddingUp = fgetc(fp);
							font_object->blockInfo->paddingRight = fgetc(fp);
							font_object->blockInfo->paddingDown = fgetc(fp);
							font_object->blockInfo->paddingLeft = fgetc(fp);
							font_object->blockInfo->spacingHoriz = fgetc(fp);
							font_object->blockInfo->spacingVert = fgetc(fp);
							font_object->blockInfo->outline = fgetc(fp);
							
							//The font's name takes up the rest of the block, so read the rest of the block as the font name
							uint32_t nameSize = size - 14;
							if ((font_object->blockInfo->fontName = (char*)malloc(nameSize)) == NULL)
							{
								error = true;
								break;
							}
							fread(font_object->blockInfo->fontName, 1, nameSize, fp);
							break;
						}
						case 2: //Common
						{
							if ((font_object->blockCommon = (BMF_Common*)malloc(sizeof(BMF_Common))) == NULL)
							{
								error = true;
								break;
							}
							font_object->blockCommon->lineHeight = File_ReadLE16(fp);
							font_object->blockCommon->base = File_ReadLE16(fp);
							font_object->blockCommon->scaleW = File_ReadLE16(fp);
							font_object->blockCommon->scaleH = File_ReadLE16(fp);
							font_object->blockCommon->pages = File_ReadLE16(fp);
							font_object->blockCommon->bitField = fgetc(fp);
							font_object->blockCommon->alphaChnl = fgetc(fp);
							font_object->blockCommon->redChnl = fgetc(fp);
							font_object->blockCommon->greenChnl = fgetc(fp);
							font_object->blockCommon->blueChnl = fgetc(fp);
							break;
						}
						case 3: //Pages
						{
							//NOTE: Common block holds how many pages there are, so that must be read before us
							if ((font_object->blockPages = (BMF_Pages*)malloc(sizeof(BMF_Pages))) == NULL || font_object->blockCommon == NULL)
							{
								error = true;
								break;
							}
							
							//Get number of page names, all page names must be the same length
							size_t nameSize = size / font_object->blockCommon->pages;
							
							//Allocate and separate all page names
							if ((font_object->blockPages->pageNames = (char**)malloc(font_object->blockCommon->pages * sizeof(char*))) == NULL)
							{
								error = true;
								break;
							}
							
							for (size_t i = 0; i < font_object->blockCommon->pages; i++)
							{
								//Allocate this name and read from file
								if ((font_object->blockPages->pageNames[i] = (char*)malloc(nameSize)) == NULL)
								{
									error = true;
									break;
								}
								fread(font_object->blockPages->pageNames[i], 1, nameSize, fp);
								if (nameSize > 4)
								{
									char *d = (char*)(font_object->blockPages->pageNames[i] + nameSize - 5);
									if (d[0] == '.' && d[1] == 't' && d[2] == 'g' && d[3] == 'a')
									{
										d[1] = 'b';
										d[2] = 'm';
										d[3] = 'p';
									}
								}
							}
							
							break;
						}
						case 4: //Chars
						{
							if ((font_object->blockChars = (BMF_Chars*)malloc(sizeof(BMF_Chars))) == NULL)
							{
								error = true;
								break;
							}
							
							//Get how many chars to allocate and read
							font_object->blockChars->numChars = size / 20;
							if ((font_object->blockChars->chars = (BMF_Char*)malloc(font_object->blockChars->numChars * sizeof(BMF_Char))) == NULL)
							{
								error = true;
								break;
							}
							
							//Read each and every char
							for (size_t i = 0; i < font_object->blockChars->numChars; i++)
							{
								BMF_Char *currentChar = &font_object->blockChars->chars[i];
								currentChar->id = File_ReadLE32(fp);
								currentChar->x = File_ReadLE16(fp);
								currentChar->y = File_ReadLE16(fp);
								currentChar->width = File_ReadLE16(fp);
								currentChar->height = File_ReadLE16(fp);
								currentChar->xoffset = File_ReadLE16(fp);
								currentChar->yoffset = File_ReadLE16(fp);
								currentChar->xadvance = File_ReadLE16(fp);
								currentChar->page = fgetc(fp);
								currentChar->chnl = fgetc(fp);
							}
							break;
						}
						case 5: //Kerning pairs
						{
							if ((font_object->blockKerningPairs = (BMF_KerningPairs*)malloc(sizeof(BMF_KerningPairs))) == NULL)
							{
								error = true;
								break;
							}
							
							//Get how many kerning pairs to allocate and read
							font_object->blockKerningPairs->numKerningPairs = size / 10;
							if ((font_object->blockKerningPairs->kerningPairs = (BMF_KerningPair*)malloc(font_object->blockKerningPairs->numKerningPairs * sizeof(BMF_KerningPair))) == NULL)
							{
								error = true;
								break;
							}
							
							//Read each and every kerning pair
							for (size_t i = 0; i < font_object->blockKerningPairs->numKerningPairs; i++)
							{
								BMF_KerningPair *currentKerningPair = &font_object->blockKerningPairs->kerningPairs[i];
								currentKerningPair->first = File_ReadLE32(fp);
								currentKerningPair->second = File_ReadLE32(fp);
								currentKerningPair->amount = File_ReadLE16(fp);
							}
							break;
						}
						default:
						{
							error = true;
							break;
						}
					}
				}
			}
			else
			{
				error = true;
			}
			
			//If not all required blocks were allocated, error
			if (font_object->blockCommon == NULL || font_object->blockPages == NULL || font_object->blockChars == NULL)
				error = true;
			
			//Close and continue with font loading if not failed
			fclose(fp);
			
			if (error == false)
			{
				//Allocate pages
				if ((font_object->pages = (RenderBackend_Surface**)malloc(font_object->blockCommon->pages * sizeof(RenderBackend_Surface*))) != NULL)
				{
					//Create and load all of our pages
					memset(font_object->pages, 0, font_object->blockCommon->pages * sizeof(RenderBackend_Surface*));
					
					for (size_t i = 0; i < font_object->blockCommon->pages; i++)
					{
						//Attempt to load the page as a bitmap
						std::string path = font_object->pagePath + font_object->blockPages->pageNames[i];
						
						unsigned int width, height;
						unsigned char *image_buffer = DecodeBitmapFromFile(path.c_str(), &width, &height);
						
						if (image_buffer != NULL)
						{
							//Create surface
							if ((font_object->pages[i] = RenderBackend_CreateSurface(width, height, FALSE)) == NULL)
							{
								error = true;
								FreeBitmap(image_buffer);
								break;
							}
							
							//Lock surface
							unsigned int pitch;
							unsigned char *pixels = RenderBackend_LockSurface(font_object->pages[i], &pitch, width, height);
							
							//Copy loaded image to surface
							for (int y = 0; y < height; ++y)
							{
								const unsigned char *src_row = &image_buffer[y * width * BITMAP_BPP];
								unsigned char *dst_row = &pixels[y * pitch];
								memcpy(dst_row, src_row, width * BITMAP_BPP);
							}
							
							//Unlock surface and free bitmap
							RenderBackend_UnlockSurface(font_object->pages[i], width, height);
							FreeBitmap(image_buffer);
						}
						else
						{
							error = true;
							break;
						}
					}
					
					if (error == false)
						return font_object;
				}
			}
		}
		
		UnloadFont(font_object);
	}
	
	return NULL;
}

//Font surface restoration
void RestoreFontSurfaces(FontObject *font_object)
{
	if (font_object != NULL && font_object->blockCommon != NULL && font_object->blockPages != NULL)
	{
		//Restore and reload every page
		for (size_t i = 0; i < font_object->blockCommon->pages; i++)
		{
			if (font_object->pages[i] != NULL)
			{
				//Attempt to load the page as a bitmap
				std::string path = font_object->pagePath + font_object->blockPages->pageNames[i];
				
				unsigned int width, height;
				unsigned char *image_buffer = DecodeBitmapFromFile(path.c_str(), &width, &height);
				
				if (image_buffer != NULL)
				{
					//Restore surface
					RenderBackend_RestoreSurface(font_object->pages[i]);
					
					//Lock surface
					unsigned int pitch;
					unsigned char *pixels = RenderBackend_LockSurface(font_object->pages[i], &pitch, width, height);
					
					//Copy loaded image to surface
					for (int y = 0; y < height; ++y)
					{
						const unsigned char *src_row = &image_buffer[y * width * BITMAP_BPP];
						unsigned char *dst_row = &pixels[y * pitch];
						memcpy(dst_row, src_row, width * BITMAP_BPP);
					}
					
					//Unlock surface
					RenderBackend_UnlockSurface(font_object->pages[i], width, height);
				}
				else
				{
					break;
				}
			}
		}
	}
}

//UTF8 to Unicode converter
static unsigned long UTF8ToUnicode(const unsigned char *string, unsigned int *bytes_read)
{
	unsigned int length;
	unsigned long charcode;

	unsigned int zero_bit = 0;
	for (unsigned char lead_byte = string[0]; zero_bit < 5 && (lead_byte & 0x80); ++zero_bit, lead_byte <<= 1);

	switch (zero_bit)
	{
		case 0:
			// Single-byte character
			length = 1;
			charcode = string[0];
			break;

		case 2:
		case 3:
		case 4:
			length = zero_bit;
			charcode = string[0] & ((1 << (8 - zero_bit)) - 1);

			for (unsigned int i = 1; i < zero_bit; ++i)
			{
				if ((string[i] & 0xC0) == 0x80)
				{
					charcode <<= 6;
					charcode |= string[i] & ~0xC0;
				}
				else
				{
					// Error: Invalid continuation byte
					length = 1;
					charcode = 0xFFFD;
					break;
				}
			}

			break;

		default:
			// Error: Invalid lead byte
			length = 1;
			charcode = 0xFFFD;
			break;

	}

	if (bytes_read)
		*bytes_read = length;

	return charcode;
}

//Draw text to a given surface (or screen) and coordinate
void DrawText(FontObject *font_object, RenderBackend_Surface *surface, int x, int y, unsigned long colour, const char *string)
{
	if (font_object != NULL && font_object->blockCommon != NULL && font_object->blockChars != NULL && font_object->pages != NULL)
	{
		//Set colour modulation of each page
		for (uint16_t i = 0; i < font_object->blockCommon->pages; i++)
			RenderBackend_SetSurfaceColorMod(font_object->pages[i], (unsigned char)colour, (unsigned char)(colour >> 8), (unsigned char)(colour >> 16));
		
		//Draw each character
		int pen_x = 0;
		
		const unsigned char *string_pointer = (unsigned char*)string;
		const unsigned char *string_end = (unsigned char*)string + strlen(string);
		
		unsigned long unicode_value = 0, last_unicode_value = 0;

		while (string_pointer != string_end)
		{
			unsigned int bytes_read;
			unicode_value = UTF8ToUnicode(string_pointer, &bytes_read);
			string_pointer += bytes_read;
			
			//Find the character that matches this value and draw it
			for (size_t i = 0; i < font_object->blockChars->numChars; i++)
			{
				BMF_Char *current_char = &font_object->blockChars->chars[i];
				if (current_char->id == unicode_value)
				{
					//Get the surface that corresponds to this character
					RenderBackend_Surface *page = font_object->pages[current_char->page];
					
					//Check for kerning pairs
					if (font_object->blockKerningPairs != NULL)
					{
						for (size_t v = 0; i < font_object->blockKerningPairs->numKerningPairs; v++)
						{
							BMF_KerningPair *pair = &font_object->blockKerningPairs->kerningPairs[v];
							if (pair->first == last_unicode_value && pair->second == unicode_value)
							{
								pen_x -= pair->amount;
								break;
							}
						}
					}
					
					//Draw character
					RenderBackend_Rect rcChar = {current_char->x, current_char->y, current_char->x + current_char->width, current_char->y + current_char->height};
					RenderBackend_Blit(page, &rcChar, surface, x + pen_x + current_char->xoffset, y + current_char->yoffset, TRUE);
					
					//Offset pen_x
					pen_x += current_char->xadvance;
					break;
				}
			}
			
			last_unicode_value = unicode_value;
		}
	}
}

unsigned int GetFontTextWidth(FontObject *font_object, const char *string)
{
	unsigned int width = 0;
	
	if (font_object != NULL && font_object->blockCommon != NULL && font_object->blockChars != NULL && font_object->pages != NULL)
	{
		//Iterate through each character
		int pen_x = 0;
		int left = 0, right = 0;
		
		const unsigned char *string_pointer = (unsigned char*)string;
		const unsigned char *string_end = (unsigned char*)string + strlen(string);
		
		unsigned long unicode_value = 0, last_unicode_value = 0;

		while (string_pointer != string_end)
		{
			unsigned int bytes_read;
			unicode_value = UTF8ToUnicode(string_pointer, &bytes_read);
			string_pointer += bytes_read;
			
			//Find the character that matches this value and draw it
			for (size_t i = 0; i < font_object->blockChars->numChars; i++)
			{
				BMF_Char *current_char = &font_object->blockChars->chars[i];
				if (current_char->id == unicode_value)
				{
					//Get the surface that corresponds to this character
					RenderBackend_Surface *page = font_object->pages[current_char->page];
					
					//Check for kerning pairs
					if (font_object->blockKerningPairs != NULL)
					{
						for (size_t v = 0; i < font_object->blockKerningPairs->numKerningPairs; v++)
						{
							BMF_KerningPair *pair = &font_object->blockKerningPairs->kerningPairs[v];
							if (pair->first == last_unicode_value && pair->second == unicode_value)
							{
								pen_x -= pair->amount;
								break;
							}
						}
					}
					
					//Update left / right and set next pen_x
					if (pen_x + current_char->xoffset < left)
						left = pen_x + current_char->xoffset;
					if (pen_x + current_char->xoffset + current_char->width > right)
						right = pen_x + current_char->xoffset + current_char->width;
					pen_x += current_char->xadvance;
					break;
				}
			}
			
			last_unicode_value = unicode_value;
		}
		
		//Get width
		width = right - left;
	}
	
	return width;
}

void UnloadFont(FontObject *font_object)
{
	if (font_object != NULL)
	{
		//Free all page surfaces
		if (font_object->pages != NULL)
		{
			if (font_object->blockCommon != NULL)
			{
				for (size_t i = 0; i < font_object->blockCommon->pages; i++)
				{
					if (font_object->pages[i] != NULL)
						RenderBackend_FreeSurface(font_object->pages[i]);
				}
			}
			free(font_object->pages);
		}
		
		//Free kerning pairs block
		if (font_object->blockKerningPairs != NULL)
		{
			free(font_object->blockKerningPairs->kerningPairs);
			free(font_object->blockKerningPairs);
		}
		
		//Free chars block
		if (font_object->blockChars != NULL)
		{
			free(font_object->blockChars->chars);
			free(font_object->blockChars);
		}
		
		//Free pages block
		if (font_object->blockPages != NULL)
		{
			if (font_object->blockPages->pageNames != NULL)
			{
				if (font_object->blockCommon != NULL)
				{
					for (size_t i = 0; i < font_object->blockCommon->pages; i++)
						free(font_object->blockPages->pageNames[i]);
				}
				free(font_object->blockPages->pageNames);
			}
			free(font_object->blockPages);
		}
		
		//Free common block
		free(font_object->blockCommon);
		
		//Free info block
		if (font_object->blockInfo != NULL)
		{
			free(font_object->blockInfo->fontName);
			free(font_object->blockInfo);
		}
		
		//Free font object
		free(font_object);
	}
}
