#include "Music.h"

#include <stdio.h>
#include <stddef.h>

#include "Main.h"
#include "File.h"
#include "Sound.h"
#include "Backends/Audio.h"

#include "Music/Organya.h"

//Music base class
class Music
{
	protected:
		const char *name = nullptr;
		
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
			//Initialize Organya
			org.InitializeData();
		}
		
		//Music interface
		bool Load(const char *_name) override
		{
			name = _name;
			std::string path = FindFilePath((gDataPath + "/Org/" + name + ".org").c_str());
			if (org.Load(path))
				return false;
			return true;
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
			if (org.IsPlaying())
				org.Mix(stream, frequency, len);
		}
};

//Music interface
Music *music = nullptr;

void Music_Callback(int32_t *stream, unsigned long frequency, size_t len)
{
	music->Mix(stream, frequency, len);
}

BOOL LoadMusic(const char *name)
{
	AudioBackend_Lock();
	if (!music->Load(name))
	{
		AudioBackend_Unlock();
		return FALSE;
	}
	AudioBackend_Unlock();
	return TRUE;
}

BOOL StartMusic()
{
	//Create music to use
	if ((music = new Music_Organya()) == nullptr)
		return FALSE;
	
	//Set music callback
	AudioBackend_SetMusicCallback(Music_Callback);
	return TRUE;
}

void EndMusic()
{
	//Stop music callback
	AudioBackend_Lock();
	AudioBackend_SetMusicCallback(Music_Callback);
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
