
#define PLATFORM_IS_ALLEGRO

/*------------------------------
		Byte order (endianness)
------------------------------*/

#if defined LITTLE_ENDIAN
//libc or something already defined the symbol we need
//so we don't need to
#elif defined BIG_ENDIAN
//libc or something already defined the symbol we need
//so we don't need to
#elif defined PLATFORM_IS_ALLEGRO
#	include <allegro.h>
#	if defined ALLEGRO_LITTLE_ENDIAN
#		define LITTLE_ENDIAN
#	elif defined ALLEGRO_BIG_ENDIAN
#		define BIG_ENDIAN
#	else
#		error endianness not defined
#	endif
#elif defined PLATFORM_IS_SDL
#	if SDL_BYTEORDER == SDL_BIG_ENDIAN
#		define BIG_ENDIAN
#	elif SDL_BYTEORDER == SDL_LITTLE_ENDIAN
#		define LITTLE_ENDIAN
#	else
#		error endianness not defined
#	endif
#else
#	if  defined(__i386__) || defined(__ia64__) || defined(WIN32) || (defined(__alpha__) || defined(__alpha)) || defined(__arm__) || (defined(__mips__) && defined(__MIPSEL__)) || defined(__LITTLE_ENDIAN__)
#		define LITTLE_ENDIAN
#	else
#		define BIG_ENDIAN
#	endif
#endif


#if defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN)
#	error Endian detection has failed (both big & little detected)
#elif !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
#	error Endian detection has failed (neither big nor little detected)
#endif

int invert_ordering (int in) {
	return (((in>>0)&0xff)<<24) + (((in>>8)&0xff)<<16) + 
			(((in>>16)&0xff)<<8) + (((in>>24)&0xff)<<0);
	}
short invert_ordering_short (short in) {
	return (((in>>0)&0xff)<<8) + (((in>>8)&0xff)<<0) ;
	}

#if defined LITTLE_ENDIAN
	int intel_ordering(int in) { return in; }
	int motorola_ordering(int in) { return invert_ordering(in); }
	short intel_ordering_short(short in) { return in; }
	short motorola_ordering_short(short in) { return invert_ordering_short(in); }
#elif defined BIG_ENDIAN
	int intel_ordering(int in) { return invert_ordering(in); }
	int motorola_ordering(int in) { return in; }
	short intel_ordering_short(short in) { return invert_ordering_short(in); }
	short motorola_ordering_short(short in) { return in; }
#else
#	error no endianness defined
#endif


