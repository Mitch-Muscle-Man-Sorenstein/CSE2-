//Standard library
#include <algorithm>
#include <iostream>

//Cave Story
#include "../Filesystem.h"
#include "../File.h"

//Declaration
#include "Organya.h"

//Organya namespace
namespace Organya
{
	//Audio buffer class
	//Destructor
	Buffer::~Buffer()
	{
		//Delete data
		delete[] data;
	}
	
	//Audio buffer interface
	bool Buffer::SetData(int8_t *_data, size_t _size, unsigned int _frequency)
	{
		//Delete previous data
		if (data != nullptr)
			delete[] data;
		
		//Copy data
		size = _size;
		if ((data = new int8_t[size + 1]) == nullptr)
			return true;
		for (size_t i = 0; i < size; i++)
			data[i] = _data[i];
		data[size] = 0;
		
		//Initialize state
		position.value = 0;
		play = false;
		loop = false;
		
		frequency = _frequency;
		volume.value = 0x100;
		volume_l.value = 0x100;
		volume_r.value = 0x100;
		pan_l.value = 0x100;
		pan_r.value = 0x100;
		return false;
	}
	
	ATTRIBUTE_HOT void Buffer::Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames)
	{
		//Don't mix if not playing or hasn't been setup yet
		if (play == false || data == nullptr)
			return;
		
		//Get sample advance delta
		uint32_t advance_delta = ((uint64_t)frequency << 16) / stream_frequency;
		
		for (size_t i = 0; i < stream_frames; i++)
		{
			//Perform linear interpolation and mix
			int16_t sample = (data[position.fixed.upper] * (0x10000 - position.fixed.lower) + data[position.fixed.upper + 1] * position.fixed.lower) >> 8;
			*stream++ += (sample * volume_l.value) >> 8;
			*stream++ += (sample * volume_r.value) >> 8;
			
			//Increment sample
			position.value += advance_delta;
			
			//Stop or loop sample once it's reached its end
			while (position.fixed.upper >= size)
			{
				if (loop)
				{
					//Move back
					position.fixed.upper -= size;
				}
				else
				{
					//Stop playing
					play = false;
					position.value = 0;
					return;
				}
			}
		}
	}
	
	void Buffer::Play(bool _loop)
	{
		play = true;
		loop = _loop;
		if (data != nullptr)
			data[size] = loop ? data[0] : 0.0f;
	}
	
	void Buffer::Stop()
	{
		play = false;
	}
	
	void Buffer::SetFrequency(unsigned int _frequency)
	{
		frequency = _frequency;
	}
	
	void Buffer::SetPosition(uint16_t _position)
	{
		position.fixed.upper = _position;
		position.fixed.lower = 0;
	}
	
	void Buffer::SetVolume(int32_t _volume)
	{
		volume.value = ConvertVolume(_volume) * 0x100;
		volume_l.value = (volume.value * pan_l.value) >> 8;
		volume_r.value = (volume.value * pan_r.value) >> 8;
	}
	
	void Buffer::SetPan(int32_t pan)
	{
		pan_l.value = ConvertVolume(-pan) * 0x100;
		pan_r.value = ConvertVolume(pan) * 0x100;
		volume_l.value = (volume.value * pan_l.value) >> 8;
		volume_r.value = (volume.value * pan_r.value) >> 8;
	}
	
	//Playback tables
	static const struct
	{
		int16_t wave_size;
		int16_t oct_par;
		int16_t oct_size;
	} oct_wave[8] =
	{
		{256, 1,   4 }, // 0 Oct
		{256, 2,   8 }, // 1 Oct
		{128, 4,   12}, // 2 Oct
		{128, 8,   16}, // 3 Oct
		{64,  16,  20}, // 4 Oct
		{32,  32,  24}, // 5 Oct
		{16,  64,  28}, // 6 Oct
		{8,   128, 32}, // 7 Oct
	};
	
	static const int16_t freq_tbl[12] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};
	static const int16_t pan_tbl[13] = {0, 43, 86, 129, 172, 215, 256, 297, 340, 383, 426, 469, 512};
	
	//Instrument class
	//Destructor
	Instrument::~Instrument()
	{
		//Delete linked list
		while (event != nullptr)
		{
			Event *next = event->next;
			delete event;
			event = next;
		}
	}
	
	//Instrument interface
	void Instrument::Reset()
	{
		//Reset instrument state
		event_point = nullptr;
		event_state = {};
	}
	
	void Instrument::SetPosition(uint32_t _x)
	{
		//If event point is null, search from beginning
		if (event_point == nullptr)
		{
			if ((event_point = event) == nullptr)
				return;
			x = 0;
		}
		
		if (_x > x)
		{
			//Move forward until we find the event to play
			while (event_point->next != nullptr && _x >= event_point->next->x)
				event_point = event_point->next;
		}
		else if (_x < x)
		{
			//Move backward until we find the event to play
			while (event_point->prev != nullptr && _x < event_point->x)
				event_point = event_point->prev;
		}
		
		//Remember given position
		x = _x;
	}
	
	void Instrument::SetVolume(signed int _volume)
	{
		//Use the given volume
		volume = _volume;
	}
	
	void Instrument::GetState()
	{
		//Don't do anything if no event could be played
		if (event == nullptr)
			return;
		if (event_point == nullptr)
			SetPosition(x);
		
		//Reset state
		event_state = {};
		
		//Go backward filling last event parameters until we hit a note
		Event *event_check = event_point;
		if (x < event_check->x)
			return;
		
		while (event_check != nullptr)
		{
			//Update volume
			if (event_check->volume != 0xFF && event_state.volume == 0xFF)
				event_state.volume = event_check->volume;
			
			//Update volume
			if (event_check->pan != 0xFF && event_state.pan == 0xFF)
				event_state.pan = event_check->pan;
			
			//Update key
			if (event_check->y != 0xFF)
			{
				event_state.x = event_check->x;
				event_state.y = event_check->y;
				
				if (x >= (event_check->x + event_check->length))
				{
					event_state.length = 0;
					Stop();
				}
				else
				{
					event_state.length = event_check->length - (x - event_check->x);
					Play();
					Update();
				}
				break;
			}
			
			//Check previous event
			event_check = event_check->prev;
		}
	}
	
	void Instrument::UpdateState()
	{
		//Don't do anything if no event could be played
		if (event == nullptr || event_point == nullptr)
			return;
			
		//Stop once length reaches 0
		if (event_state.length != 0 && --event_state.length == 0)
			Stop();
		
		//If new event has played, update state
		if (x == event_point->x)
		{
			//Update and play key
			if (event_point->y != 0xFF)
			{
				event_state.y = event_point->y;
				event_state.length = event_point->length;
				Play();
			}
			
			//Update volume
			if (event_point->volume != 0xFF)
				event_state.volume = event_point->volume;
			
			//Update pan
			if (event_point->pan != 0xFF)
				event_state.pan = event_point->pan;
			
			//Update buffers
			Update();
		}
	}
	
	//Melody class
	bool Melody::ConstructBuffers(const Instance &organya)
	{
		for (size_t i = 0; i < 8; i++)
		{
			//Get sizes
			size_t wave_size = oct_wave[i].wave_size, data_size = wave_size * (pipi ? oct_wave[i].oct_size : 1);
			
			//Allocate buffer data
			int8_t *data = new int8_t[data_size];
			if (data == nullptr)
				return true;
			int8_t *datap = data;
			
			//Get wave position
			const int8_t *wave;
			if ((wave = organya.GetWave(wave_no)) == nullptr)
				return true;
			
			//Fill buffer data
			size_t wave_tp = 0;
			for (size_t j = 0; j < data_size; j++)
			{
				*datap++ = wave[wave_tp];
				wave_tp = (wave_tp + (0x100 / wave_size)) & 0xFF;
			}
			
			//Create buffers
			for (size_t v = 0; v < 2; v++)
				if (buffer[i][v].SetData(data, data_size, 22050))
					return true;
			delete[] data;
		}
		return false;
	}
	
	void Melody::StopBuffers()
	{
		for (size_t i = 0; i < 8; i++)
			for (size_t v = 0; v < 2; v++)
				buffer[i][v].Stop();
		current_buffer = nullptr;
		twin = false;
	}
	
	void Melody::Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames)
	{
		for (size_t i = 0; i < 8; i++)
			for (size_t v = 0; v < 2; v++)
				buffer[i][v].Mix(stream, stream_frequency, stream_frames);
	}
	
	void Melody::Play()
	{
		//Stop current buffer and switch to new one
		if (current_buffer != nullptr)
			current_buffer->Play(false);
		twin = !twin;
		current_buffer = &buffer[event_state.y / 12][twin];
		current_buffer->Play(!pipi);
	}
	
	void Melody::Update()
	{
		if (event_state.y != 0xFF)
		{
			//Update frequency for.. every buffer? WHAT? WHY?!
			for (size_t i = 0; i < 8; i++)
				for (size_t v = 0; v < 2; v++)
					buffer[i][v].SetFrequency(((oct_wave[i].wave_size * freq_tbl[event_state.y % 12]) * oct_wave[i].oct_par) / 8 + (freq - 1000));
		}
		
		//If not playing, don't update
		if (current_buffer == nullptr)
			return;
		
		//Update volume and panning for current buffer
		current_buffer->SetVolume(((event_state.volume * volume / 0x7F) - 0xFF) * 8);
		if (event_state.pan != 0xFF)
			current_buffer->SetPan((pan_tbl[event_state.pan] - 256) * 10);
	}
	
	void Melody::Stop()
	{
		//If not playing, don't stop
		if (current_buffer == nullptr)
			return;
		current_buffer->Play(false);
		current_buffer = nullptr;
	}
	
	//Drum class
	bool Drum::ConstructBuffers(const Instance &organya)
	{
		PXT_SND *drum = &gPxtSnd[150 + wave_no];
		if (drum->data != nullptr && buffer.SetData(drum->data, drum->size, 22050))
			return true;
		return false;
	}
	
	void Drum::StopBuffers()
	{
		buffer.Stop();
	}
	
	void Drum::Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames)
	{
		buffer.Mix(stream, stream_frequency, stream_frames);
	}
	
	void Drum::Play()
	{
		//Rewind and play buffer
		buffer.Play(false);
		buffer.SetFrequency(100 + (unsigned int)event_state.y * 800);
		buffer.SetPosition(0.0);
	}
	
	void Drum::Update()
	{
		//Update volume and panning for buffer
		buffer.SetVolume(((event_state.volume * volume / 0x7F) - 0xFF) * 8);
		if (event_state.pan != 0xFF)
			buffer.SetPan((pan_tbl[event_state.pan] - 256) * 10);
	}
	
	void Drum::Stop()
	{
		//Nothing
		return;
	}
	
	//Organya instance class
	//Destructor
	Instance::~Instance()
	{
		return;
	}
	
	//Data initialization
	bool Instance::InitializeData()
	{
		//Open and read waveforms
		FILE *fp = OpenFile(FSS_Mod, "wave.dat", "rb");
		if (!fp)
			return true;
		
		int8_t *wavep = &wave[0][0];
		for (size_t i = 0; i < 0x100 * 100; i++)
			*wavep++ = (int8_t)fgetc(fp);
		return false;
	}
	
	//Loading
	struct VersionLUT
	{
		std::string name;
		Version version;
	};
	
	static const std::array<VersionLUT, 3> version_lut{{
		{"Org-01", OrgV01},
		{"Org-02", OrgV02},
	}};
	
	bool Instance::ReadInstrument(std::istream &stream, Instrument &i, uint16_t &note_num)
	{
		//Read instrument data
		i.freq = ReadLE16(stream);
		if ((i.wave_no = stream.get()) >= 100)
			return true;
		i.pipi = stream.get();
		if (header.version == OrgV01)
			i.pipi = false; //OrgV01 has no pipi, but the byte is still there
		note_num = ReadLE16(stream);
		return false;
	}
	
	bool Instance::ReadEvents(std::istream &stream, Instrument &i, uint16_t note_num)
	{
		//Construct note linked list
		Event *event;
		Event *prev = nullptr;
		
		for (uint16_t v = 0; v < note_num; v++)
		{
			//Create a new event instance and link it to the list
			if ((event = new Event) == nullptr)
				return true;
			if ((event->prev = prev) != nullptr)
				prev->next = event; //Link us to previous event
			else
				i.event = event; //Be head of instrument events if first
			prev = event;
		}
		
		//Read data into notes
		for (event = i.event; event != nullptr; event = event->next)
			event->x = ReadLE32(stream);
		for (event = i.event; event != nullptr; event = event->next)
			if ((event->y = stream.get()) >= 12 * 8 && event->y != 0xFF)
				return true;
		for (event = i.event; event != nullptr; event = event->next)
			event->length = stream.get();
		for (event = i.event; event != nullptr; event = event->next)
			event->volume = stream.get();
		for (event = i.event; event != nullptr; event = event->next)
			event->pan = stream.get();
		return false;
	}
	
	bool Instance::Load(std::istream &stream)
	{
		//Unload previous data
		Unload();
		
		//Read magic and version
		std::string magic = ReadString<6>(stream);
		for (auto &i : version_lut)
		{
			if (i.name == magic)
			{
				header.version = i.version;
				break;
			}
		}
		
		//Read rest of header
		header.wait = ReadLE16(stream);
		header.line = stream.get();
		header.dot = stream.get();
		header.repeat_x = ReadLE32(stream);
		header.end_x = ReadLE32(stream);
		
		//Read instrument data
		uint16_t note_num[16];
		uint16_t *note_num_p = note_num;
		
		for (auto &i : melody)
			if (ReadInstrument(stream, i, *note_num_p++))
				return true;
		for (int i = 0; i < drum.size(); i++)
		{
			if (ReadInstrument(stream, drum[i], *note_num_p++))
				return true;
			drum[i].wave_no = i;
		}
		
		//Construct instrument buffers
		for (auto &i : melody)
			if (i.ConstructBuffers(*this))
				return true;
		for (auto &i : drum)
			if (i.ConstructBuffers(*this))
				return true;
		
		//Read event data
		note_num_p = note_num;
		for (auto &i : melody)
			if (ReadEvents(stream, i, *note_num_p++))
				return true;
		for (auto &i : drum)
			if (ReadEvents(stream, i, *note_num_p++))
				return true;
		
		//Reset state
		volume = 100;
		fading = false;
		SetPosition(0);
		return false;
	}
	
	bool Instance::Load(std::string _path)
	{
		//Get the stream that represents the path given, and remember the path for later saving
		std::ifstream stream(_path);
		if (stream.is_open())
			return Load(stream);
		else
			return true;
	}
	
	//Other internal functions
	void Instance::UnloadInstrument(Instrument &i)
	{
		//Delete linked list
		while (i.event != nullptr)
		{
			Event *next = i.event->next;
			delete i.event;
			i.event = next;
		}
		i.Reset();
	}
	
	void Instance::Unload()
	{
		//Unload instruments
		for (auto &i : melody)
			UnloadInstrument(i);
		for (auto &i : drum)
			UnloadInstrument(i);
	}
	
	//Organya interface
	bool Instance::SetPosition(uint32_t _x)
	{
		//Set all instrument positions and get their states
		x = _x;
		
		for (auto &i : melody)
		{
			i.SetPosition(x);
			i.SetVolume(volume);
			i.GetState();
		}
		for (auto &i : drum)
		{
			i.SetPosition(x);
			i.SetVolume(volume);
			i.GetState();
		}
		
		//Reset step frames counter
		step_frames_counter = 0;
		return false;
	}
	
	bool Instance::Play()
	{
		if (!playing)
		{
			//Start playing
			playing = true;
			SetPosition(x);
		}
		return false;
	}
	
	bool Instance::Stop()
	{
		if (playing)
		{
			//Stop playing
			playing = false;
			for (auto &i : melody)
				i.StopBuffers();
			for (auto &i : drum)
				i.StopBuffers();
		}
		return false;
	}
	
	//Mixing interface
	void Instance::Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames)
	{
		//Update and mix Organya
		size_t step_frames = header.wait * stream_frequency / 1000;
		size_t frames_done = 0;
		
		while (frames_done < stream_frames)
		{
			if (step_frames_counter >= step_frames)
			{
				//Reset step frames counter
				step_frames_counter = 0;
				
				//Increment position in song
				if (++x >= header.end_x)
					x = header.repeat_x;
				
				//Handle fading out
				if (fading && volume > 0)
				{
					if ((volume -= 2) < 0)
						volume = 0;
				}
				
				//Update instruments
				for (auto &i : melody)
				{
					i.SetPosition(x);
					i.SetVolume(volume);
					i.UpdateState();
				}
				for (auto &i : drum)
				{
					i.SetPosition(x);
					i.SetVolume(volume);
					i.UpdateState();
				}
			}
			
			//Get frames to do
			size_t frames_to_do = stream_frames - frames_done;
			if (frames_to_do > step_frames - step_frames_counter)
				frames_to_do = step_frames - step_frames_counter;
			
			//Mix instruments
			for (auto &i : melody)
				i.Mix(stream, stream_frequency, frames_to_do);
			for (auto &i : drum)
				i.Mix(stream, stream_frequency, frames_to_do);
			stream += frames_to_do * 2;
			frames_done += frames_to_do;
			step_frames_counter += frames_to_do;
		}
	}
	
	int16_t *Instance::MixToBuffer(unsigned int frequency, size_t frames)
	{
		//Allocate 32-bit buffer to mix into before clamping
		int32_t *buffer;
		if ((buffer = new int32_t[frames * 2]{}) == nullptr)
			return nullptr;
		
		//Mix to mix buffer
		Mix(buffer, frequency, frames);
		
		//Allocate final buffer
		int16_t *fbuffer;
		if ((fbuffer = new int16_t[frames * 2]) == nullptr)
		{
			delete[] buffer;
			return nullptr;
		}
		int16_t *fbufferp = fbuffer;
		
		//Clamp and write buffer to final buffer
		int32_t *bufferp = buffer;
		for (size_t i = 0; i < frames * 2; i++)
		{
			if (*bufferp < -0x7FFF)
				*fbufferp++ = -0x7FFF;
			else if (*bufferp > 0x7FFF)
				*fbufferp++ = 0x7FFF;
			else
				*fbufferp++ = *bufferp;
			bufferp++;
		}
		
		//Delete mix buffer
		delete[] buffer;
		return fbuffer;
	}
	
	bool Instance::MixToStream(std::ostream &stream, unsigned int frequency, size_t frames)
	{
		//Get mixed buffer
		int16_t *buffer;
		if ((buffer = MixToBuffer(frames, frequency)) == nullptr)
			return true;
		
		//Write mixed buffer to stream
		int16_t *bufferp = buffer;
		for (size_t i = 0; i < frames * 2; i++)
			WriteLE16(stream, *bufferp++);
		delete[] buffer;
		return false;
	}
}
