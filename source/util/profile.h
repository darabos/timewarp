#ifndef _PROFILE_H
#define _PROFILE_H

#define PROFILE_LEVEL             5
#define PROFILE_STACKTRACE_LEVEL  5

#define PROFILE_TIME_TYPE1 Sint64
#define PROFILE_TIME_TYPE2 Sint64
#define PROFILE_TIME_FUNC() get_time3()

#define _PROFILE_STACKTRACE STACKTRACE

struct SOURCE_LINE;

struct DIRECT_PROFILE_DATUM {
	SOURCE_LINE *srcline;
	int runs;
	PROFILE_TIME_TYPE2 time;
};

void _register_profile_datum ( DIRECT_PROFILE_DATUM *data );

struct Profiler {
	PROFILE_TIME_TYPE1 time;
	DIRECT_PROFILE_DATUM &data;
	INLINE Profiler ( DIRECT_PROFILE_DATUM &_data) : time((PROFILE_TIME_TYPE1)PROFILE_TIME_FUNC()), data(_data) {
	}
	INLINE ~Profiler() {
		if (!data.runs) _register_profile_datum ( &data );
		data.runs++;
		data.time += (PROFILE_TIME_TYPE2) (PROFILE_TIME_FUNC() - time);
	}
};

#define _PROFILE static SOURCE_LINE _srcline = { __LINE__, __FILE__, NULL }; static DIRECT_PROFILE_DATUM _profiler_data_ = { &_srcline, 0, 0 }; const Profiler _profiler_ ## __LINE__ ( _profiler_data_ ) ;
#define _PROFILE_(A) static SOURCE_LINE _srcline = { __LINE__, __FILE__, A }; static DIRECT_PROFILE_DATUM _profiler_data_ = { &_srcline, 0, 0 }; const Profiler _profiler_ ## __LINE__ ( _profiler_data_ ) ;
//#define _PROFILE_STACKTRACE static ProfileData _profiler_data_ ## __LINE__ ( __FILE__, __LINE__ ); const Profiler _profiler_ ## __LINE__ ( & _profiler_data_ ## __LINE__ ) ;


#if PROFILE_STACKTRACE_LEVEL >= 1
#	define PROFILE_STACKTRACE1 _PROFILE_STACKTRACE1
#else
#	define PROFILE1
#endif
#if PROFILE_STACKTRACE_LEVEL >= 2
#	define PROFILE_STACKTRACE2 _PROFILE_STACKTRACE2
#else
#	define PROFILE2
#endif
#if PROFILE_STACKTRACE_LEVEL >= 3
#	define PROFILE_STACKTRACE3 _PROFILE_STACKTRACE3
#else
#	define PROFILE3
#endif
#if PROFILE_STACKTRACE_LEVEL >= 4
#	define PROFILE_STACKTRACE4 _PROFILE_STACKTRACE4
#else
#	define PROFILE4
#endif
#if PROFILE_STACKTRACE_LEVEL >= 5
#	define PROFILE_STACKTRACE5 _PROFILE_STACKTRACE5
#else
#	define PROFILE5
#endif


#if PROFILE_LEVEL >= 1
#	define PROFILE1 _PROFILE
#else
#	define PROFILE1
#endif
#if PROFILE_LEVEL >= 2
#	define PROFILE2 _PROFILE
#else
#	define PROFILE2
#endif
#if PROFILE_LEVEL >= 3
#	define PROFILE3 _PROFILE
#else
#	define PROFILE3
#endif
#if PROFILE_LEVEL >= 4
#	define PROFILE4 _PROFILE
#else
#	define PROFILE4
#endif
#if PROFILE_LEVEL >= 5
#	define PROFILE5 _PROFILE
#else
#	define PROFILE5
#endif

//*/
#endif
