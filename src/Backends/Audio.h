#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct AudioBackend_Sound AudioBackend_Sound;

typedef void (*MusicCallback)(int32_t *stream, unsigned long frequency, size_t len);

bool AudioBackend_Init(void);
void AudioBackend_Deinit(void);

AudioBackend_Sound* AudioBackend_CreateSound(unsigned int frequency, const signed char *samples, size_t length);
void AudioBackend_DestroySound(AudioBackend_Sound *sound);

void AudioBackend_PlaySound(AudioBackend_Sound *sound, bool looping);
void AudioBackend_StopSound(AudioBackend_Sound *sound);
void AudioBackend_RewindSound(AudioBackend_Sound *sound);

void AudioBackend_SetSoundFrequency(AudioBackend_Sound *sound, unsigned int frequency);
void AudioBackend_SetSoundVolume(AudioBackend_Sound *sound, long volume);
void AudioBackend_SetSoundPan(AudioBackend_Sound *sound, long pan);

void AudioBackend_Lock();
void AudioBackend_Unlock();

void AudioBackend_SetMusicCallback(MusicCallback callback);
