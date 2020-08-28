#include "Music.h"

#include <stdio.h>
#include <stddef.h>
#include <iostream>

#include "Main.h"
#include "File.h"
#include "Filesystem.h"
#include "Sound.h"
#include "Backends/Audio.h"

#include "Music/Organya.h"
#include "Music/Ogg.h"

//Music type
MusicType gMusicType;

//Music base class
class Music
{
	protected:
		const char *name;
		
	public:
		virtual ~Music() {}
		
		//Music interface
		virtual bool Load(const char *_name) = 0;
		const char *GetName() { return name; }
		
		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual bool IsPlaying() = 0;
		
		virtual void SetPosition(unsigned int x) = 0;
		virtual unsigned int GetPosition() = 0;
		
		virtual void SetVolume(signed int volume) = 0;
		virtual signed int GetVolume() = 0;
		virtual void SetFadeout() = 0;
		virtual bool GetFadeout() = 0;
		
		virtual void Mix(int32_t *stream, unsigned long frequency, size_t len) = 0;
};

//Organya music class
class Music_Organya : public Music
{
	private:
		//Organya
		Organya::Instance org;
		
	public:
		//Constructor
		Music_Organya()
		{
			name = nullptr;
			org.InitializeData();
		}
		
		//Music interface
		bool Load(const char *_name) override
		{
			std::string path = FindFile(FSS_Mod, std::string("Org/") + _name + ".org");
			if (org.Load(path))
			{
				name = nullptr;
				return true;
			}
			name = _name;
			return false;
		}
		
		void Play() override
		{
			if (name)
				org.Play();
		}
		
		void Stop() override
		{
			if (name)
				org.Stop();
		}
		
		bool IsPlaying() override
		{
			if (name)
				return org.IsPlaying();
			return false;
		}
		
		void SetPosition(unsigned int x) override
		{
			if (name)
				org.SetPosition(x);
		}
		
		unsigned int GetPosition() override
		{
			if (name)
				return org.GetPosition();
			return 0;
		}
		
		void SetVolume(signed int volume) override
		{
			if (name)
				org.SetVolume(volume);
		}
		
		signed int GetVolume() override
		{
			if (name)
				return org.GetVolume();
			return 100;
		}
		
		void SetFadeout() override
		{
			if (name)
				org.SetFadeout();
		}
		
		bool GetFadeout() override
		{
			if (name)
				return org.GetFadeout();
			return false;
		}
		
		void Mix(int32_t *stream, unsigned long frequency, size_t len) override
		{
			if (name && org.IsPlaying())
				org.Mix(stream, frequency, len);
		}
};

//Ogg base music class
class Music_Ogg : public Music
{
	private:
		//Ogg
		Ogg ogg;
		
		//Music folder
		std::string folder;
		
	public:
		//Constructor
		Music_Ogg(std::string _folder)
		{
			name = nullptr;
			folder = _folder + '/';
		}
		
		//Music interface
		bool Load(const char *_name) override
		{
			if (ogg.Load(folder + _name))
			{
				name = nullptr;
				return true;
			}
			name = _name;
			return false;
		}
		
		void Play() override
		{
			if (name)
				ogg.Play();
		}
		
		void Stop() override
		{
			if (name)
				ogg.Stop();
		}
		
		bool IsPlaying() override
		{
			if (name)
				return ogg.IsPlaying();
			return false;
		}
		
		void SetPosition(unsigned int x) override
		{
			if (name)
				ogg.SetPosition(x);
		}
		
		unsigned int GetPosition() override
		{
			if (name)
				return ogg.GetPosition();
			return 0;
		}
		
		void SetVolume(signed int volume) override
		{
			if (name)
				ogg.SetVolume(volume);
		}
		
		signed int GetVolume() override
		{
			if (name)
				return ogg.GetVolume();
			return 100;
		}
		
		void SetFadeout() override
		{
			if (name)
				ogg.SetFadeout();
		}
		
		bool GetFadeout() override
		{
			if (name)
				return ogg.GetFadeout();
			return false;
		}
		
		void Mix(int32_t *stream, unsigned long frequency, size_t len) override
		{
			if (name && ogg.IsPlaying())
				ogg.Mix(stream, frequency, len);
		}
};

//Music interface
Music *music = nullptr;

void Music_Callback(int32_t *stream, unsigned long frequency, size_t len)
{
	music->Mix(stream, frequency, len);
}

void SetMusicType(MusicType type)
{
	if (type != gMusicType)
	{
		AudioBackend_Lock();
		
		//Delete previous music class and create new one
		delete music;
		switch (gMusicType = type)
		{
			case MT_Organya:
				music = new Music_Organya();
				break;
			case MT_Ogg:
				music = new Music_Ogg("Ogg");
				break;
			case MT_Ogg11:
				music = new Music_Ogg("Ogg11");
				break;
			default:
				music = nullptr;
				break;
		}
		
		AudioBackend_Unlock();
	}
}

BOOL LoadMusic(const char *name)
{
	AudioBackend_Lock();
	
	//Load given song
	if (music->Load(name))
	{
		AudioBackend_Unlock();
		return FALSE;
	}
	
	AudioBackend_Unlock();
	return TRUE;
}

BOOL StartMusic()
{
	//Set music type
	SetMusicType(MT_Ogg11);
	
	//Set music callback
	AudioBackend_SetMusicCallback(Music_Callback);
	return TRUE;
}

void EndMusic()
{
	//Stop music callback
	AudioBackend_Lock();
	AudioBackend_SetMusicCallback(nullptr);
	AudioBackend_Unlock();
	
	//Delete music
	delete music;
}

void PlayMusic()
{
	AudioBackend_Lock();
	music->Play();
	AudioBackend_Unlock();
}

void StopMusic()
{
	AudioBackend_Lock();
	music->Stop();
	AudioBackend_Unlock();
}

void SetMusicPosition(unsigned int x)
{
	AudioBackend_Lock();
	music->SetPosition(x);
	AudioBackend_Unlock();
}

unsigned int GetMusicPosition()
{
	AudioBackend_Lock();
	unsigned int x = music->GetPosition();
	AudioBackend_Unlock();
	return x;
}

BOOL ChangeMusicVolume(signed int volume)
{
	AudioBackend_Lock();
	music->SetVolume(volume);
	AudioBackend_Unlock();
	return TRUE;
}

void SetMusicFadeout()
{
	AudioBackend_Lock();
	music->SetFadeout();
	AudioBackend_Unlock();
}
