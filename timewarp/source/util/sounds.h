#ifndef _SOUNDS_H
#define _SOUNDS_H

#define PLATFORM_IS_ALLEGRO

#ifdef PLATFORM_IS_ALLEGRO
	struct SAMPLE;
	struct JGMOD;
	typedef SAMPLE  Sound;
	typedef JGMOD   Music;
#else
	#error unknown platform (allegro?)
#endif

class SoundSystem : public BaseClass {
	public:
	enum {
		ENABLED = 1,
		DISABLED = 2,
		MOD_ENABLED = 4,
		MOD_DISABLED = 8, 
		SOUND_ON = 16,
		MUSIC_ON = 32
		};
	Music *looping_music;
	char fake_mod_playing;
	char sound_on, music_on;
	char sound_channels, music_channels;
	int sound_volume, music_volume;
	int state;

	SoundSystem() ;
	void disable() ;

	void load() ;
	void save() ;
	void init() ;

	int play (Sound *spl, int vol = 256, int pan = 128, int freq = 1000) ;
	void stop (Sound *spl);

	void play_music (Music *mus, int loop = false);
	void stop_music ();
	int is_music_playing() const;
	int is_music_supported() const;

	void set_volumes(int sound_volume, int music_volume, int sound_on, int music_on) ;

	Music *load_music(const char *fname);
	void unload_music(Music *music);

}
extern tw_soundsystem;
#define sound tw_soundsystem
#endif
