#pragma once

//Standard library
#include <cstdint>
#include <string>
#include <fstream>
#include <array>
#include <math.h>

//Cave Story
#include "../Sound.h"
#include "../Attributes.h"

//Organya namespace
namespace Organya
{
	#pragma pack(push)
	#pragma pack(1)
	template <typename T, typename TU, typename TL>
	union FixedPoint
	{
		#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			struct
			{
				TL lower;
				TU upper;
			} fixed;
		#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
			struct
			{
				TU upper;
				TL lower;
			} fixed;
		#endif
		T value;
	};
	#pragma pack(pop)

	//Audio buffer class
	class Buffer
	{
		private:
			//Audio data
			int8_t *data = nullptr;
			size_t size;
			FixedPoint<uint32_t, uint16_t, uint16_t> position;
			
			//Audio state
			bool play = false, loop = false;
			unsigned int frequency;
			FixedPoint<uint16_t, uint8_t, uint8_t> volume, volume_l, volume_r, pan_l, pan_r;
			
		public:
			//Constructors and destructor
			Buffer() { }
			Buffer(int8_t *_data, size_t _size, int _frequency) { SetData(_data, _size, _frequency); };
			~Buffer();
			
			//Buffer interface
			bool SetData(int8_t *_data, size_t _size, unsigned int _frequency);
			
			ATTRIBUTE_HOT void Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames);
			
			void Play(bool _loop);
			void Stop();
			
			void SetPosition(uint16_t _position);
			void SetFrequency(unsigned int _frequency);
			void SetVolume(int32_t _volume);
			void SetPan(int32_t pan);
			
		private:
			//Volume convertion
			float ConvertVolume(int32_t volume)
			{
				if (volume < -10000)
					volume = -10000;
				else if (volume > 0)
					volume = 0;
				return powf(10.0f, volume / 2000.0f);
			}
	};
	
	class Instance; //prototype
	
	//Organya types
	enum Version
	{
		OrgV01, //Original
		OrgV02, //Added pipi
	};

	struct Header
	{
		Version version;	//Organya version
		uint16_t wait;				//Tempo
		uint8_t line, dot;			//Time signature
		uint32_t repeat_x;			//Repeat
		uint32_t end_x;				//End of song (Return to repeat)
	};

	struct Event
	{
		//Event information
		uint32_t x = 0;			//Position
		uint8_t length = 0;		//Length
		uint8_t y = 0xFF;		//Key
		uint8_t volume = 0xFF;	//Volume
		uint8_t pan = 0xFF;		//Panning
		
		//Next and previous event (double sided linked list)
		Event *prev = nullptr;
		Event *next = nullptr;
	};

	class Instrument
	{
		public:
			//Instrument info
			uint16_t freq;		//Frequency offset (1000 is 0hz offset)
			uint8_t wave_no;	//Waveform number
			bool pipi;			//Pizzicato
			
			//Music data
			Event *event = nullptr;	//All the events that belong to this instrument
			
		protected:
			//Instrument state
			Event *event_point = nullptr; //Pointer to the current event being played
			Event event_state; //Current event state (key, volume, and pan are changed individually)
			
			//Playback state
			signed int volume;
			uint32_t x;
			
		public:
			//Destructor
			~Instrument();
			
			//Instrument interface
			void Reset();
			
			virtual bool ConstructBuffers(const Instance &organya) = 0;
			virtual void StopBuffers() = 0;
			virtual void Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames) = 0;
			
			void SetPosition(uint32_t _x);
			void SetVolume(signed int _volume);
			void GetState();
			void UpdateState();
			
		private:
			//Internal instrument interface
			virtual void Play() = 0;
			virtual void Update() = 0;
			virtual void Stop() = 0;
	};

	class Melody : public Instrument
	{
		private:
			//Sound buffers for playback
			Buffer buffer[8][2] = {};
			Buffer *current_buffer = nullptr;
			bool twin = false;
			
		public:
			//Instrument interface
			bool ConstructBuffers(const Instance &organya);
			void StopBuffers();
			void Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames);
			
		private:
			//Internal instrument interface
			void Play() override;
			void Update() override;
			void Stop() override;
	};

	class Drum : public Instrument
	{
		private:
			//Sound buffer for playback
			Buffer buffer{};
			
		public:
			//Instrument interface
			bool ConstructBuffers(const Instance &organya);
			void StopBuffers();
			void Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames);
			
		protected:
			//Internal instrument interface
			void Play() override;
			void Update() override;
			void Stop() override;
	};
	
	//Organya instance class
	class Instance
	{
		private:
			//Waveforms
			int8_t wave[100][0x100];
			
			//Organya data
			Header header;
			std::array<Melody, 8> melody = {};
			std::array<Drum, 8> drum = {};
			
			//Playback state
			bool playing = false;
			
			signed int volume = 100;
			bool fading = false;
			
			unsigned int step_frames_counter;
			uint32_t x;
			
		public:
			//Constructor and destructor
			Instance() {}
			Instance(std::istream &stream) { Load(stream); }
			Instance(std::string _path) { Load(_path); }
			~Instance();
			
			//Data initialization
			bool InitializeData();
			
			//Loading, saving, and creation
			bool Load(std::istream &stream);
			bool Load(std::string _path);
			
			//Organya interface
			bool SetPosition(uint32_t x);
			bool Play();
			bool Stop();
			
			bool IsPlaying() const { return playing; };
			uint32_t GetPosition();
			
			void SetVolume(signed int _volume);
			signed int GetVolume();
			void SetFadeout();
			bool GetFadeout() { return fading; }
			
			//Mixing interface
			void Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames);
			int16_t *MixToBuffer(unsigned int frequency, size_t frames);
			bool MixToStream(std::ostream &stream, unsigned int frequency, size_t frames);
			
			//Internal Organya interface
			const int8_t *GetWave(uint8_t wave_no) const
			{
				if (wave_no >= 100)
					return nullptr;
				return wave[wave_no];
			}
			
		private:
			//Internal loading functions
			bool ReadInstrument(std::istream &stream, Instrument &i, uint16_t &note_num);
			bool ReadEvents(std::istream &stream, Instrument &i, uint16_t note_num);
			
			//Other internal functions
			void UnloadInstrument(Instrument &i);
			void Unload();
			
			//Read string of specified length from file
			template<unsigned length> std::string ReadString(std::istream &stream)
			{
				char buf[length + 1] = {0};
				stream.read(buf, length);
				return std::string(buf);
			}
			
			//Read specific sizes from stream
			uint16_t ReadLE16(std::istream &stream)
			{ return ((uint16_t)stream.get()) | ((uint16_t)stream.get() << 8); }
			uint32_t ReadLE32(std::istream &stream)
			{ return ((uint32_t)stream.get()) | ((uint32_t)stream.get() << 8) | ((uint32_t)stream.get() << 16) | ((uint32_t)stream.get() << 24); }
			
			//Write specific sizes from stream
			void WriteLE16(std::ostream &stream, uint16_t x)
			{ stream.put((uint8_t)(x >> 0)); stream.put((uint8_t)(x >> 8)); }
			void WriteLE32(std::ostream &stream, uint32_t x)
			{ stream.put((uint8_t)(x >> 0)); stream.put((uint8_t)(x >> 8)); stream.put((uint8_t)(x >> 16)); stream.put((uint8_t)(x >> 24)); }
	};
}
