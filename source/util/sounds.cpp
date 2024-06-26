/* $Id$ */ 

//#define NO_JGMOD

#include <allegro.h>

#include "base.h"
#include "sounds.h"

//#include "../melee.h"
#include "../melee/mframe.h"

//#if defined DIGI_DIRECTAMX
//#	define DIGI_TW DIGI_DIRECTAMX(0)
//#else 
#	define DIGI_TW DIGI_AUTODETECT
//#endif

#define MIDI_TW MIDI_NONE

#define MAX_SOUND_CHANNELS 10
#define MIN_SOUND_CHANNELS 4
#define MAX_MUSIC_CHANNELS 22
#define MIN_MUSIC_CHANNELS 10


/* static int is_jgmod_compiled() {return 0;} */


SoundSystem tw_soundsystem;
SoundSystem::SoundSystem() {
	sound_volume = 255;
	music_volume = 255;
	sound_on = false;
	music_on = false;
	state = 0;
	fake_mod_playing = false;
	return;
}
int SoundSystem::is_music_supported() const {
#	ifdef NO_JGMOD
	return 0;
#	else
	return 1;
#	endif
}
int SoundSystem::is_music_playing() const {
	if (state & MOD_ENABLED) return ::is_mod_playing();
	else return fake_mod_playing;
}
void SoundSystem::disable() {
	if (state & ENABLED) {
		if (state & MOD_ENABLED) {
			::remove_mod();
		}
		::remove_sound();
	}
	state &= ~ENABLED;
	state |= DISABLED;
	sound_channels = 0;
	music_channels = 0;
	return;
}
void SoundSystem::load() {
	int sv, mv, so, mo;
	so = get_config_int("Sound", "SoundOn", sound_on);
	mo = get_config_int("Sound", "MusicOn", music_on);
	sv = get_config_int("Sound", "SoundVolume", sound_volume);
	mv = get_config_int("Sound", "MusicVolume", music_volume);
	set_volumes(sv, mv, so, mo);
	return;
}
void SoundSystem::save() {
	set_config_int("Sound", "SoundOn", sound_on);
	set_config_int("Sound", "MusicOn", music_on);
	set_config_int("Sound", "SoundVolume", sound_volume);
	set_config_int("Sound", "MusicVolume", music_volume);
	return;
}
void SoundSystem::init() {
	if (state & (ENABLED | DISABLED)) return;
	state |= DISABLED;
	int voices = detect_digi_driver(DIGI_TW);

	if (voices <= 0) return;
	if (voices > MAX_SOUND_CHANNELS + MAX_MUSIC_CHANNELS) voices = MAX_SOUND_CHANNELS + MAX_MUSIC_CHANNELS;
	sound_channels = voices;
	music_channels = 0;
	if ((voices < MIN_MUSIC_CHANNELS + MIN_SOUND_CHANNELS) || (state & MOD_DISABLED)) {
	}
	else {
		music_channels = (MIN_MUSIC_CHANNELS + voices - MIN_SOUND_CHANNELS) / 2;
		if (sound_channels > MAX_SOUND_CHANNELS) sound_channels = MAX_SOUND_CHANNELS;
		music_channels = voices - sound_channels;
		if (music_channels > MAX_MUSIC_CHANNELS) music_channels = MAX_MUSIC_CHANNELS;
		sound_channels = voices - music_channels;
	}
	if (sound_channels > MAX_SOUND_CHANNELS) sound_channels = MAX_SOUND_CHANNELS;
	::set_volume_per_voice(2);

	::reserve_voices(sound_channels + music_channels, 0);
	if (install_sound(DIGI_TW, MIDI_TW, "") < 0) {

		sound_channels = 0;
		music_channels = 0;
		return;
	}

	//	::set_volume(255, 255);
	state &=~DISABLED;
	state |= ENABLED;
	if (music_channels > 0) {
		::install_mod( music_channels );
		::set_mod_volume(music_volume);

		state |= MOD_ENABLED;
	}
	return;
}

void check_sample_duration(SAMPLE *spl, int freq, bool loop)
{
	if (!physics)
		return;

	if (loop)
	{
		throw("a looped option for a temporary SAMPLE is not allowed");
	} else {

		// frequency modifier.
		double factor = double(freq) / 1000.0;

		// duration in seconds.
		double duration = double(spl->len) / (double(spl->freq) * factor);

		// available time in seconds.
		double time_available = DEATH_FRAMES * physics->frame_time * 1E-3;
		if ( duration > time_available )
		{
			throw("the sample might disappear too soon; increase DEATH_FRAMES");
		}
	}

}

int SoundSystem::play (SAMPLE *spl, int vol, int pan, int freq, bool loop, bool noerrorcheck) {
	if (spl) {
		if ((state & (ENABLED | SOUND_ON)) == (ENABLED | SOUND_ON))
		{
			//if (freq > 4535) freq = 4535;
			//I THINK that the 4536 bug is specific to my sound hardware, so that's commented out
			if (!noerrorcheck)
				check_sample_duration(spl, freq, loop);
			return ::play_sample (spl, (vol * sound_volume) >> 8, pan, freq, loop);
		} else {
			return -1;
		}
	}
}
void SoundSystem::stop (SAMPLE *spl) {
	if (spl) {
		if (state & ENABLED) {
			::stop_sample (spl);
			return;
		}
		else return;
	}
}
void SoundSystem::stop (int voice_id) {
	if (voice_id < 0) {throw("negative voice id");}
	if (state & ENABLED) {
		::voice_stop (voice_id);
		return;
	}
	else return;
}


void check_music_duration(JGMOD *music, bool loop)
{
	if (!physics)
		return;

	if (loop)
	{
		throw("a looped option for a temporary JGMOD is not allowed");
	} else {
		double time_available = DEATH_FRAMES * physics->frame_time * 1E-3;

		// I do not know how to calculate the duration of a mod...
		// so I'll just be on the safe side, with 20 seconds or so.
		if (time_available < 20.0)
		{
			throw("the mod music might disappear too soon; increase DEATH_FRAMES");
		}
	}

}

void SoundSystem::play_music (Music *music, int loop, bool do_error_check) {
	if ((state & (MOD_ENABLED | MUSIC_ON)) == (MOD_ENABLED | MUSIC_ON))
	{
		if (do_error_check)
			check_music_duration(music, loop);

		::play_mod(music, loop);
	}

	fake_mod_playing = loop;
	if (fake_mod_playing)
		looping_music = music;
	return;
}
void SoundSystem::stop_music () {
	if (state & MOD_ENABLED)
		::stop_mod();
	fake_mod_playing = false;
	return;
}
void SoundSystem::set_volumes(int sound_volume, int music_volume, int sound_on, int music_on) {
	if (!(state & (ENABLED | DISABLED))) {
		//error("sound system not initialized");
		init();
	}
	this->sound_volume = sound_volume & 255;
	this->music_volume = music_volume & 255;
	this->sound_on = sound_on;
	this->music_on = music_on;
	state &= ~(SOUND_ON | MUSIC_ON);
	if (sound_on) state |= SOUND_ON;
	if (music_on) state |= MUSIC_ON;
	if (state & MOD_ENABLED) {
		set_mod_volume (music_volume);
		if (!(state & MUSIC_ON)) ::stop_mod();
		else if (fake_mod_playing && !is_music_playing()) play_music(looping_music, true);
	}
	return;
}

Music *SoundSystem::load_music(const char *fname) 
{
	return load_mod((char*)fname);
}

void SoundSystem::unload_music(Music *mus) 
{
	destroy_mod(mus);
}
