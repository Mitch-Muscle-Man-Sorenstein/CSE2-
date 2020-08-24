#pragma once
#include "WindowsWrapper.h"
#include <stddef.h>

void Music_Callback(long *stream, unsigned long frequency, size_t len);

BOOL LoadMusic(const char *name);

BOOL StartMusic();
void EndMusic();

void PlayMusic();
void StopMusic();

void SetMusicPosition(unsigned int x);
unsigned int GetMusicPosition();

BOOL ChangeMusicVolume(signed int volume);
void SetMusicFadeout();
