/* $Id$ */ 
#ifndef __SOUNDS_H__
#define __SOUNDS_H__

#define PLATFORM_IS_ALLEGRO

#ifdef PLATFORM_IS_ALLEGRO
	struct SAMPLE;
	struct JGMOD;
	typedef SAMPLE  Sound;
	typedef JGMOD   Music;
#else
	#error unknown platform (allegro?)
#endif

#include "base.h"

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

	int play (Sound *spl, int vol = 256, int pan = 128, int freq = 1000, bool loop = false, bool noerrorcheck = false) ;
	void stop (Sound *spl);
	void stop (int voice_id);

	void play_music (Music *mus, int loop = false, bool do_error_check = true);
	void stop_music ();
	int is_music_playing() const;
	int is_music_supported() const;

	void set_volumes(int sound_volume, int music_volume, int sound_on, int music_on) ;

	Music *load_music(const char *fname);
	void unload_music(Music *music);

}
extern tw_soundsystem;
#define sound tw_soundsystem

/*------------------------------
		Sound (and MODs)
------------------------------*/
#ifdef NO_JGMOD
	static void set_mod_volume(int a) {}
	static void play_mod(Music *a, int b) {}
	static void stop_mod() {}
	static bool is_mod_playing() {return false;}
	static void remove_mod() {}
	static void install_mod(int a) {}
	static void pause_mod() {}
	static void resume_mod() {}
	static Music *load_mod(const char *fname) {return NULL;}
	static void destroy_mod(Music *) {}
#	undef MIN_MUSIC_CHANNELS
#	undef MAX_MUSIC_CHANNELS
#	define MIN_MUSIC_CHANNELS 0
#	define MAX_MUSIC_CHANNELS 0
#else
  #include <jgmod.h>
#endif


#endif // __SOUNDS_H__
