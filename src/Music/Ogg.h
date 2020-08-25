#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <string>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.cpp"
#undef STB_VORBIS_HEADER_ONLY
#include "miniaudio.h"

struct OggData
{
	std::vector<stb_vorbis*> decoder;
	size_t current_decoder = 0;
	unsigned int frequency;
	unsigned int channels;
	
	void CloseDecoders()
	{
		for (auto &i : decoder)
			stb_vorbis_close(i);
		decoder.clear();
		current_decoder = 0;
	}
};

class Ogg
{
	private:
		//Loaded song data
		OggData data;
		
		//Resampler
		ma_pcm_converter_config config;
		ma_pcm_converter resampler;
		
		//Playback state
		bool playing;
		
	public:
		//Constructors and destructor
		Ogg() {}
		Ogg(std::string name) { Load(name); }
		~Ogg();
		
		//Loading
		bool Load(std::string name);
		
		//Ogg interface
		bool Play();
		bool Stop();
		bool IsPlaying() { return playing; }
		
		bool SetPosition(uint32_t x);
		uint32_t GetPosition();
		
		//Mixing interface
		void Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames);
		
	private:
		//Internal loading functions
		bool LoadTrack(FILE *file);
};
