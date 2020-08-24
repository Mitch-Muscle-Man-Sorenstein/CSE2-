#pragma once
#include "Draw.h"
#include "Profile.h"

void Title_PutSave(PROFILE *profile, int y, BOOL selected);
void Title_PutCenterText(int x, int y, const char *text, unsigned long color);
void Title_PutBox(const RECT *rect, BOOL selected);
void Title_PutTextBox(int x, int y, const char *text, BOOL selected);
void Title_PutTextBox2(int x, int y, const char *text, BOOL selected);
void Title_PutCenterTextBox(int x, int y, const char *text, BOOL selected);
