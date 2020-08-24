#pragma once

#include <stddef.h>

#include "Backends/Rendering.h"

typedef struct FontObject FontObject;

FontObject* LoadFontFromData(const unsigned char *data, size_t data_size, unsigned int cell_width, unsigned int cell_height);
FontObject* LoadFont(const char *font_filename, unsigned int cell_width, unsigned int cell_height);
void RestoreFontSurfaces(FontObject *font_object);
void DrawText(FontObject *font_object, RenderBackend_Surface *surface, int x, int y, unsigned long colour, const char *string);
unsigned int GetFontTextWidth(FontObject *font_object, const char *string);
void UnloadFont(FontObject *font_object);
