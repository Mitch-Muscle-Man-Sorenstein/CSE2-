// Some of the original source code for this file can be found here:
// https://github.com/shbow/organya/blob/master/source/Sound.cpp

#include "Sound.h"

#include <stddef.h>
#include <stdio.h>	// Used by commented-out code
#include <stdlib.h>
#include <string.h>

#include "WindowsWrapper.h"

#include "Backends/Audio.h"
#include "Main.h"
#include "Music.h"
#include "PixTone.h"
#include "Music.h"

BOOL audio_backend_initialised;
AudioBackend_Sound *lpSECONDARYBUFFER[SE_MAX];
PXT_SND gPxtSnd[SE_MAX] = {};

// DirectSoundの開始 (Starting DirectSound)
BOOL InitDirectSound(void)
{
	int i;

	audio_backend_initialised = AudioBackend_Init();

	if (!audio_backend_initialised)
		return FALSE;
	
	for (i = 0; i < SE_MAX; i++)
	{
		lpSECONDARYBUFFER[i] = NULL;
		gPxtSnd[i].data = nullptr;
	}

	StartMusic();
	return TRUE;
}

// DirectSoundの終了 (Exit DirectSound)
void EndDirectSound(void)
{
	int i;

	//End and free sound data
	EndMusic();

	for (i = 0; i < SE_MAX; i++)
	{
		if (lpSECONDARYBUFFER[i] != NULL)
			AudioBackend_DestroySound(lpSECONDARYBUFFER[i]);
		free(gPxtSnd[i].data);
	}

	//Quit audio backend
	if (!audio_backend_initialised)
		return;
	AudioBackend_Deinit();
}

void PlaySoundObject(int no, int mode)
{
	if (!audio_backend_initialised)
		return;

	if (lpSECONDARYBUFFER[no] != NULL)
	{
		switch (mode)
		{
			case 0:	// 停止 (Stop)
				AudioBackend_StopSound(lpSECONDARYBUFFER[no]);
				break;

			case 1:	// 再生 (Playback)
				AudioBackend_StopSound(lpSECONDARYBUFFER[no]);
				AudioBackend_RewindSound(lpSECONDARYBUFFER[no]);
				AudioBackend_PlaySound(lpSECONDARYBUFFER[no], FALSE);
				break;

			case -1:// ループ再生 (Loop playback)
				AudioBackend_PlaySound(lpSECONDARYBUFFER[no], TRUE);
				break;
		}
	}
}

void ChangeSoundFrequency(int no, unsigned long rate)	// 100がMIN9999がMAXで2195?がﾉｰﾏﾙ (100 is MIN, 9999 is MAX, and 2195 is normal)
{
	if (!audio_backend_initialised)
		return;
	if (lpSECONDARYBUFFER[no] != NULL)
		AudioBackend_SetSoundFrequency(lpSECONDARYBUFFER[no], (rate * 10) + 100);
}

void ChangeSoundVolume(int no, long volume)	// 300がMAXで300がﾉｰﾏﾙ (300 is MAX and 300 is normal)
{
	if (!audio_backend_initialised)
		return;
	if (lpSECONDARYBUFFER[no] != NULL)
		AudioBackend_SetSoundVolume(lpSECONDARYBUFFER[no], (volume - 300) * 8);
}

void ChangeSoundPan(int no, long pan)	// 512がMAXで256がﾉｰﾏﾙ (512 is MAX and 256 is normal)
{
	if (!audio_backend_initialised)
		return;
	if (lpSECONDARYBUFFER[no] != NULL)
		AudioBackend_SetSoundPan(lpSECONDARYBUFFER[no], (pan - 256) * 10);
}

// TODO - The stack frame for this function is inaccurate
int MakePixToneObject(const PIXTONEPARAMETER *ptp, int ptp_num, int no)
{
	int sample_count;
	int i, j;
	const PIXTONEPARAMETER *ptp_pointer;
	unsigned char *pcm_buffer;
	unsigned char *mixed_pcm_buffer;

	if (!audio_backend_initialised)
		return 0;

	ptp_pointer = ptp;
	sample_count = 0;

	for (i = 0; i < ptp_num; i++)
	{
		if (ptp_pointer->size > sample_count)
			sample_count = ptp_pointer->size;

		++ptp_pointer;
	}

	pcm_buffer = mixed_pcm_buffer = NULL;

	pcm_buffer = (unsigned char*)malloc(sample_count);
	mixed_pcm_buffer = (unsigned char*)malloc(sample_count);

	if (pcm_buffer == NULL || mixed_pcm_buffer == NULL)
	{
		free(pcm_buffer);
		free(mixed_pcm_buffer);
		return -1;
	}

	memset(pcm_buffer, 0x80, sample_count);
	memset(mixed_pcm_buffer, 0x80, sample_count);

	ptp_pointer = ptp;

	for (i = 0; i < ptp_num; i++)
	{
		if (!MakePixelWaveData(ptp_pointer, pcm_buffer))
		{
			if (pcm_buffer != NULL) // This is always true
				free(pcm_buffer);

			if (mixed_pcm_buffer != NULL) // This is always true
				free(mixed_pcm_buffer);

			return -1;
		}

		for (j = 0; j < ptp_pointer->size; j++)
		{
			if (pcm_buffer[j] + mixed_pcm_buffer[j] - 0x100 < -0x7F)
				mixed_pcm_buffer[j] = 0;
			else if (pcm_buffer[j] + mixed_pcm_buffer[j] - 0x100 > 0x7F)
				mixed_pcm_buffer[j] = 0xFF;
			else
				mixed_pcm_buffer[j] = mixed_pcm_buffer[j] + pcm_buffer[j] - 0x80;
		}

		++ptp_pointer;
	}

	// Create backend sound
	if (lpSECONDARYBUFFER[no] != NULL)
		AudioBackend_DestroySound(lpSECONDARYBUFFER[no]);
	lpSECONDARYBUFFER[no] = AudioBackend_CreateSound(22050, mixed_pcm_buffer, sample_count);
	free(pcm_buffer);

	if (no >= 150)
	{
		//Convert sound to signed 8 bit and store it for later usage
		uint8_t *mpb = mixed_pcm_buffer;
		for (int i = 0; i < sample_count; i++)
			*mpb++ -= 0x80;
		
		PXT_SND *snd = &gPxtSnd[no];
		snd->data = (int8_t*)mixed_pcm_buffer;
		snd->size = sample_count;
	}
	else
	{
		free(mixed_pcm_buffer);
	}

	if (lpSECONDARYBUFFER[no] == NULL)
		return -1;

	return sample_count;
}
