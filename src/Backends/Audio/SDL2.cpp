#include "../Audio.h"

#include <stddef.h>
#include <string.h>
#include <string>

#include "SDL.h"

#include "../Misc.h"

#include "SoftwareMixer.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static SDL_AudioDeviceID device_id;

static unsigned long output_frequency;

MusicCallback music_callback;

static void Callback(void *user_data, Uint8 *stream_uint8, int len)
{
	(void)user_data;

	short *stream = (short*)stream_uint8;
	const size_t frames_total = len / sizeof(short) / 2;

	size_t frames_done = 0;

	while (frames_done != frames_total)
	{
		int32_t mix_buffer[0x800 * 2];	// 2 because stereo

		size_t subframes = MIN(0x800, frames_total - frames_done);

		memset(mix_buffer, 0, subframes * sizeof(int32_t) * 2);

		if (music_callback != nullptr)
			music_callback(mix_buffer, output_frequency, subframes);
		Mixer_MixSounds(mix_buffer, subframes);

		for (size_t i = 0; i < subframes * 2; ++i)
		{
			if (mix_buffer[i] > 0x7FFF)
				*stream++ = 0x7FFF;
			else if (mix_buffer[i] < -0x7FFF)
				*stream++ = -0x7FFF;
			else
				*stream++ = mix_buffer[i];
		}

		frames_done += subframes;
	}
}

bool AudioBackend_Init(void)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		std::string errorMessage = std::string("'SDL_InitSubSystem(SDL_INIT_AUDIO)' failed: ") + SDL_GetError();
		Backend_ShowMessageBox("Fatal error (SDL2 audio backend)", errorMessage.c_str());
		return false;
	}

	Backend_PrintInfo("Available SDL audio drivers:");

	for (int i = 0; i < SDL_GetNumAudioDrivers(); ++i)
		Backend_PrintInfo("%s", SDL_GetAudioDriver(i));

	SDL_AudioSpec specification;
	specification.freq = 44100;
	specification.format = AUDIO_S16;
	specification.channels = 2;
	specification.samples = 0x400;	// Roughly 10 milliseconds for 48000Hz
	specification.callback = Callback;
	specification.userdata = NULL;

	SDL_AudioSpec obtained_specification;
	device_id = SDL_OpenAudioDevice(NULL, 0, &specification, &obtained_specification, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	if (device_id == 0)
	{
		std::string error_message = std::string("'SDL_OpenAudioDevice' failed: ") + SDL_GetError();
		Backend_ShowMessageBox("Fatal error (SDL2 audio backend)", error_message.c_str());
		return false;
	}

	output_frequency = obtained_specification.freq;
	Mixer_Init(obtained_specification.freq);
	
	music_callback = nullptr;

	SDL_PauseAudioDevice(device_id, 0);

	Backend_PrintInfo("Selected SDL audio driver: %s", SDL_GetCurrentAudioDriver());

	return true;
}

void AudioBackend_Deinit(void)
{
	SDL_CloseAudioDevice(device_id);

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

AudioBackend_Sound* AudioBackend_CreateSound(unsigned int frequency, const signed char *samples, size_t length)
{
	SDL_LockAudioDevice(device_id);

	Mixer_Sound *sound = Mixer_CreateSound(frequency, samples, length);

	SDL_UnlockAudioDevice(device_id);

	return (AudioBackend_Sound*)sound;
}

void AudioBackend_DestroySound(AudioBackend_Sound *sound)
{
	if (sound == NULL)
		return;

	SDL_LockAudioDevice(device_id);

	Mixer_DestroySound((Mixer_Sound*)sound);

	SDL_UnlockAudioDevice(device_id);
}

void AudioBackend_PlaySound(AudioBackend_Sound *sound, bool looping)
{
	if (sound == NULL)
		return;

	SDL_LockAudioDevice(device_id);

	Mixer_PlaySound((Mixer_Sound*)sound, looping);

	SDL_UnlockAudioDevice(device_id);
}

void AudioBackend_StopSound(AudioBackend_Sound *sound)
{
	if (sound == NULL)
		return;

	SDL_LockAudioDevice(device_id);

	Mixer_StopSound((Mixer_Sound*)sound);

	SDL_UnlockAudioDevice(device_id);
}

void AudioBackend_RewindSound(AudioBackend_Sound *sound)
{
	if (sound == NULL)
		return;

	SDL_LockAudioDevice(device_id);

	Mixer_RewindSound((Mixer_Sound*)sound);

	SDL_UnlockAudioDevice(device_id);
}

void AudioBackend_SetSoundFrequency(AudioBackend_Sound *sound, unsigned int frequency)
{
	if (sound == NULL)
		return;

	SDL_LockAudioDevice(device_id);

	Mixer_SetSoundFrequency((Mixer_Sound*)sound, frequency);

	SDL_UnlockAudioDevice(device_id);
}

void AudioBackend_SetSoundVolume(AudioBackend_Sound *sound, long volume)
{
	if (sound == NULL)
		return;

	SDL_LockAudioDevice(device_id);

	Mixer_SetSoundVolume((Mixer_Sound*)sound, volume);

	SDL_UnlockAudioDevice(device_id);
}

void AudioBackend_SetSoundPan(AudioBackend_Sound *sound, long pan)
{
	if (sound == NULL)
		return;

	SDL_LockAudioDevice(device_id);

	Mixer_SetSoundPan((Mixer_Sound*)sound, pan);

	SDL_UnlockAudioDevice(device_id);
}

void AudioBackend_Lock()
{
	SDL_LockAudioDevice(device_id);
}

void AudioBackend_Unlock()
{
	SDL_UnlockAudioDevice(device_id);
}

void AudioBackend_SetMusicCallback(MusicCallback callback)
{
	SDL_LockAudioDevice(device_id);
	music_callback = callback;
	SDL_UnlockAudioDevice(device_id);
}
