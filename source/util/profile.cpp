
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <allegro.h>

#include "base.h"
#include "round.h"
#include "errors.h"
#include "get_time.h"
#include "profile.h"
#include "profile2.h"

#define SAFE_TIME_FUNC()  get_time()
#define SAFE_TIME_RATIO   1000.0

#include <algorithm>
using namespace std;

//static R_Array <DIRECT_PROFILE_DATUM*, 16> _raw_master_profile;
static vector<DIRECT_PROFILE_DATUM*> _raw_master_profile;



static PROFILE_TIME_TYPE2 _profile_time;
static Uint32 _safe_profile_time;
static double _time_ratio = -1;
static int _last_ratio_update = -1;
void init_profiling() {
	if (!is_time_initialized()) tw_error("init_profiling - you must call init_time() first");
	_profile_time = PROFILE_TIME_FUNC();
	_safe_profile_time = SAFE_TIME_FUNC();
	_time_ratio = 1;
	_last_ratio_update = _safe_profile_time;
}
void deinit_profiling() {
	_time_ratio = -1;
}
static void update_ratio() {
	if (_time_ratio == -1) tw_error("update_ratio - forgot to call init_profiling");
	Uint32 csafetime = SAFE_TIME_FUNC();
	if (_last_ratio_update == csafetime) return;
	PROFILE_TIME_TYPE2 cprofiletime = PROFILE_TIME_FUNC();
	PROFILE_TIME_TYPE2 dtp = (cprofiletime - _profile_time);
	Uint32 dts = (csafetime - _safe_profile_time);
	_time_ratio =  (double(dts) / double(dtp)) / SAFE_TIME_RATIO;
}

class _DPD_COMPARE_ALPHA {
	public:
	int operator() ( DIRECT_PROFILE_DATUM *a, DIRECT_PROFILE_DATUM *b ) {
		int r = strcmp(a->srcline->file, b->srcline->file);
		if (r) return r < 0;
		r = a->srcline->line - b->srcline->line;
		if (r) return r > 0;
		tw_error("this shouldn't happen... (_DPD_COMPARE)");
		return r;
	}
};

void _register_profile_datum ( DIRECT_PROFILE_DATUM *data ) {
	const char *tmp = strstr(data->srcline->file, "source");
	if (tmp) data->srcline->file = tmp + 7;
	int i = _raw_master_profile.size();
	//_raw_master_profile.grow(1);
	//_raw_master_profile[i] = data;
	_raw_master_profile.push_back(data);
	stable_sort(&_raw_master_profile[0], &_raw_master_profile[i+1], _DPD_COMPARE_ALPHA() );
}

class _PD_COMPARE_ALPHA {
	public:
	int operator() ( const PROFILE_DATUM &a, const PROFILE_DATUM &b ) {
		int r = strcmp(a.srcline->file, b.srcline->file);
		if (r) return r < 0;
		r = a.srcline->line - b.srcline->line;
		if (r) return r > 0;
		tw_error("this shouldn't happen... (_PD_COMPARE)");
		return r;
	}
};
class _PD_COMPARE_TIME {
	public:
	int operator() ( const PROFILE_DATUM &a, const PROFILE_DATUM &b ) {
		return a.time > b.time;
	}
};

Profile::Profile() : total_time(0), active(0), sorted(SORTED_ALPHA) {
}
Profile::~Profile() {
}
/*
Profile &Profile::operator=  ( const Profile &p ) {
	sorted = p.sorted;//?
	active = 0;//?
	total_time = p.total_time;
//	data.preinit();
//	int i = p.data.size();
//	data.grow(i);
//	memcpy(&data[0], &p.data[0], sizeof(PROFILE_DATUM) * i);
	data = p.data;
	return *this;
}
*/
Profile &Profile::operator+= ( const Profile &p ) {
	int i;
	int mj = 0;
	bool oo = false;
	total_time += p.total_time;
	for (i = 0; i < p.data.size(); i++) {
		int j;
		for (j = mj; j < data.size(); j++) {
			if (p.data[i].srcline == data[j].srcline) {
				if (j == mj) mj++;
				break;
			}
		}
		if (j == data.size()) {
			oo = true;
			//data.grow(1);
			data.resize(data.size()+1);
			data[j].srcline = p.data[i].srcline;
			data[j].runs = 0;
			data[j].time = 0;
		}
		data[j].runs += p.data[i].runs;
		data[j].time += p.data[i].time;
	}
	if (oo) _sort_alpha();
	return *this;
}
Profile &Profile::operator-= ( const Profile &p ) {
	int i;
	int mj = 0;
	bool oo = false;
	total_time -= p.total_time;
	for (i = 0; i < p.data.size(); i++) {
		int j;
		for (j = mj; j < data.size(); j++) {
			if (p.data[i].srcline == data[j].srcline) {
				if (j == mj) mj++;
				break;
			}
		}
		if (j == data.size()) {
			oo = true;
//			data.grow(1);
			data.resize(data.size()+1);
			data[j].srcline = p.data[i].srcline;
			data[j].runs = 0;
			data[j].time = 0;
		}
		data[j].runs -= p.data[i].runs;
		data[j].time -= p.data[i].time;
	}
	if (oo) _sort_alpha();
	return *this;
}

static Profile *master_profile = NULL;

void Profile::_sort_alpha() {
	stable_sort(&data[0], &data[data.size()], _PD_COMPARE_ALPHA() );
	sorted = SORTED_ALPHA;
}
void Profile::_sort_time() {
	stable_sort(&data[0], &data[data.size()], _PD_COMPARE_TIME() );
	sorted = SORTED_TIME;
}
void Profile::sort_alpha() {if (sorted != SORTED_ALPHA) _sort_alpha();}
void Profile::sort_time() {if (sorted != SORTED_TIME) _sort_time();}

void Profile::clear ( ) {
	int i;
	for (i = 0; i < data.size(); i++) {
		data[i].time = 0;
		data[i].runs = 0;
	}
	total_time = 0;
}
Profile *get_master_profile() {
	if (!master_profile) master_profile = new Profile();

	update_ratio();

	int i = _raw_master_profile.size();
	int j = master_profile->data.size();
	if (i < j) {
		tw_error("this shouldn't happen (gmp)");
	}
	//if (i > j) master_profile->data.grow(i-j);
	if (i > j) master_profile->data.resize(i);
	if (i == j) {
		master_profile->sort_alpha();
		for (j = 0; j < i; j++) {
			master_profile->data[j].runs = _raw_master_profile[j]->runs;
			master_profile->data[j].time = _raw_master_profile[j]->time;
		}
	}
	else {
		for (j = 0; j < i; j++) {
			master_profile->data[j].srcline = _raw_master_profile[j]->srcline;
			master_profile->data[j].runs = _raw_master_profile[j]->runs;
			master_profile->data[j].time = _raw_master_profile[j]->time;
		}
	}
	master_profile->total_time = PROFILE_TIME_FUNC() - _profile_time;
	return master_profile;
}


SelectProfile::SelectProfile ( Profile &_data ) : data(_data), temp(NULL) {
	if (&_data == NULL) tw_error("SelectProfile - NULL profile");
	if (_data.active != 0) tw_error("SelectProfile - profile already enabled");
	data.sort_alpha();
	temp = new Profile();
	data.active ++;
	*temp = *get_master_profile();
}
SelectProfile::~SelectProfile () {
	data -= *temp;
	data += *get_master_profile();
	data.sort_alpha();
	data.active --;
	if (temp) delete temp;
}

/*void Profile::print ( int i, BITMAP *surface, int x, int y, int c ) {
	char *tmp;
	tmp = source_line_to_text ( data[i].srcline );
	textprintf ( surface, font, x, y, c, "%3d%% %8d %8d %s", data[i].time * 100.0 / total_time, data[i].runs, int(data[i].time / data[i].runs / 1000), tmp);
	free(tmp);
}*/
int Profile::print ( int i, char *dest, int max ) {
	if (_time_ratio == -1) {tw_error("Profile::print - forgot to call init_profiling");}
	if ((i < 0) || (i >= data.size())) {
		dest[0] = 0;
		return -1;
	}
	const SOURCE_LINE *srcline = data[i].srcline;
	const char *tmp;
	tmp = srcline->name;//source_line_to_text ( data[i].srcline );
	int runs = data[i].runs;
	int per = iround(double(data[i].time) * 1000.0 / total_time);
	if (!runs) runs = 1;
#ifdef _MSC_VER
	int r = _snprintf ( dest, max, 
#elif defined VSNPRINTF 
	int r = snprintf ( dest, max, 
#else
	int r = sprintf ( dest, 
#endif
		"%3d.%d%%  %8d  %8d  %s", 
		per / 10, per % 10,
		iround(data[i].time / double(runs) * _time_ratio * (1000.0 * 1000.0 * 1000)), //1000ths of microseconds
		data[i].runs, 
		tmp
	);
	//free(tmp);
	return r;
}


int bob() {
	_PROFILE
	//_STACKTRACE("BOB")
//	static DIRECT_PROFILE_DATUM _profile_datum = { 1152, "fred", 0,  0, 0 };
//	UserStackTraceHelper a( (SOURCE_LINE*) &_profile_datum );
//	Profiler b ( _profile_datum );
	//STACKTRACE
	//static const SOURCE_LINE _srcline = { 1152, "fred", 0 }; UserStackTraceHelper _stacktrace_ ( &_srcline );
	return 0;
}


