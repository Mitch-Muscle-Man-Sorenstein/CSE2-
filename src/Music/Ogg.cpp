#include "Ogg.h"
#include "../File.h"

//Resampler callback
ma_uint32 MusicResampleCallback(ma_pcm_converter *dsp, void *buffer, ma_uint32 frames, void *userdata)
{
	//Get Ogg data
	OggData *data = (OggData*)userdata;
	
	//Read the given amount of samples
	int16_t *bufferp = (int16_t*)buffer;
	ma_uint32 samples = frames * 2;
	int samples_read = 0;
	
	while (samples_read < samples)
	{
		//Read data and advance through buffer
		int read = stb_vorbis_get_samples_short_interleaved(data->decoder[data->current_decoder].decoder, data->channels, bufferp, samples - samples_read) * data->channels;
		bufferp += read;
		samples_read += read;
		
		//If reached the end of the file, either loop, or if there's no loop point, stop playing
		if (read == 0)
		{
			//Get next decoder index and play it
			if (++data->current_decoder >= data->decoder.size())
				data->current_decoder--;
			stb_vorbis_seek_start(data->decoder[data->current_decoder].decoder);
			break;
		}
	}
	
	return samples_read / data->channels;
}

//Ogg song class
//Destructor
Ogg::~Ogg()
{
	//Close decoders
	data.CloseDecoders();
}

//Loading
bool Ogg::LoadTrack(FILE *file)
{
	//Open decoder
	stb_vorbis *decoder;
	int error;
	if (file == nullptr)
		return true;
	else if ((decoder = stb_vorbis_open_file(file, 1, &error, nullptr)) == nullptr)
		return true;
	
	//Get track information and push to decoder list
	Decoder dec_struct;
	
	stb_vorbis_info info = stb_vorbis_get_info(decoder);
	data.frequency = info.sample_rate;
	data.channels = info.channels;
	
	dec_struct.decoder = decoder;
	dec_struct.length = stb_vorbis_stream_length_in_samples(decoder) / data.channels;
	data.decoder.push_back(dec_struct);
	return false;
}

bool Ogg::Load(std::string name)
{
	//Reset state
	playing = false;
	
	//Close previous decoders
	data.CloseDecoders();
	
	//Open new decoders
	if (LoadTrack(FindFile((name + "_intro.ogg").c_str(), "rb")))
	{
		if (LoadTrack(FindFile((name + ".ogg").c_str(), "rb")))
			return true;
	}
	else 
	{
		if (LoadTrack(FindFile((name + "_loop.ogg").c_str(), "rb")))
			return true;
	}
	
	//Initialize resampler
	config = ma_pcm_converter_config_init(ma_format_s16, data.channels, data.frequency, ma_format_s16, 2, 0, MusicResampleCallback, (void*)&data);
	return false;
}

//Ogg interface
bool Ogg::Play()
{
	playing = true;
	volume = 100;
	fading = false;
	fade_counter = 0;
	return false;
}

bool Ogg::Stop()
{
	playing = false;
	return false;
}

bool Ogg::SetPosition(uint32_t x)
{
	size_t i = 0;
	while (x > data.decoder[i].length && ++i < data.decoder.size())
		x -= data.decoder[i-1].length;
	data.current_decoder = i;
	stb_vorbis_seek_frame(data.decoder[i].decoder, x);
	return false;
}

uint32_t Ogg::GetPosition()
{
	size_t pre_pos = 0;
	for (size_t i = 0; i < data.current_decoder; i++)
		pre_pos += data.decoder[i].length;
	return pre_pos + stb_vorbis_get_sample_offset(data.decoder[data.current_decoder].decoder);
}

//Mixing interface
void Ogg::Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames)
{
	//Fade out
	if (fading)
	{
		unsigned int fade_rate = stream_frequency / 40;
		fade_counter += stream_frames;
		while (fade_counter >= fade_rate)
		{
			if (volume > 0)
				volume--;
			fade_counter -= fade_rate;
		}
	}
	
	//Initialize resampler with our stream frequency
	if (config.sampleRateOut != stream_frequency)
	{
		config.sampleRateOut = stream_frequency;
		ma_pcm_converter_init(&config, &resampler);
	}
	
	//Resample song
	int16_t *buffer;
	if ((buffer = new int16_t[stream_frames * 2]) == nullptr)
		return;
	ma_uint64 res = ma_pcm_converter_read(&resampler, buffer, stream_frames);
	
	//Mix song
	int16_t *bufferp = buffer;
	for (size_t i = 0; i < stream_frames * 2; i++)
		*stream++ += *bufferp++ * (volume + 40) / 140;
	delete[] buffer;
	
}
