#include <stdio.h>
#include <string.h>
#include <allegro.h>

#include "../melee.h"

#include "mlog.h"
#include "mgame.h"
#include "../util/net_tcp.h"


////////////////////////////////////////////////////////////////////////
//				Logging stuff
////////////////////////////////////////////////////////////////////////

void Log::init() {STACKTRACE
	log_len  = NULL;
	log_size = NULL;
	log_pos  = NULL;
	log_data = NULL;
	log_dir = NULL;
	default_direction = direction_write | direction_read;
	log_num = 0;
	playback = false;
	type = Log::log_normal;
	return;
}

void Log::set_direction ( int channel, char direction ) {
	STACKTRACE;
	if (channel < 0) { tw_error("set_direction - channel < 0"); }
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	log_dir[channel] = direction;
	return;
}

char Log::get_direction ( int channel ) {
	STACKTRACE;
	if (channel < 0) {tw_error("get_direction - channel < 0");}
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	return (log_dir[channel]);
}

void Log::set_all_directions ( char direction ) {STACKTRACE
	default_direction = direction;
	int i;
	for (i = 0; i < log_num; i += 1) {
		log_dir[i] = direction;
	}
	return;
}

Log::~Log() {STACKTRACE
	if (log_data) for (int i = 0; i < log_num; i += 1) {
		free(log_data[i]);
	}
	free(log_len);
	free(log_size);
	free(log_pos);
	free(log_data);
	free(log_dir);
	log_num = 0;
}

void Log::log ( int channel, void *data, int size) {STACKTRACE
	if (!size) return;
	if (channel < 0) {tw_error ("Log::log - negative channel!");}
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	if (log_dir[channel] & direction_write) _log ( channel, data, size);
	if (log_dir[channel] & direction_read) _unlog ( channel, data, size);
	return;
}
void Log::_log(int channel, const void *data, int size) {STACKTRACE
	log_len[channel] += size;
	while (log_len[channel] > log_size[channel]) {
		if (log_size[channel]) log_size[channel] = log_size[channel] * 2;
		else log_size[channel] = 32;
		log_data[channel] = (unsigned char *) realloc(log_data[channel], log_size[channel]);
	}
	memcpy(log_data[channel]+(log_len[channel]-size), data, size);
	return;
}
void Log::_unlog(int channel, void *data, int size) {
	STACKTRACE;
	if (log_len[channel] < log_pos[channel] + size) {tw_error ("Game::_unlog - went past end (%d+%d/%d on %d)", log_pos[channel], size, log_len[channel], channel);}
	memcpy(data, log_data[channel]+log_pos[channel], size);
	log_pos[channel] += size;
	return;
}

void Log::save (const char *fname) {STACKTRACE
	PACKFILE *f;
	int i, j;
	f = pack_fopen ( fname, F_WRITE_PACKED);
	if (!f) throw "Log::save failed";
	j = 0;
	for (j = 0, i = 0; i < log_num; i += 1) if (log_len[i] && !(log_dir[i] & direction_forget)) j += 1;
	pack_iputl ( j, f);
	for (i = 0; i < log_num; i += 1) {
		if (!log_len[i] || (log_dir[i] & direction_forget)) continue;
		f = pack_fopen_chunk(f, 0);
		pack_iputl ( i, f);
		pack_iputl ( log_len[i], f);
		pack_fwrite ( log_data[i], log_len[i], f);
		f = pack_fclose_chunk(f);
	}
	pack_fclose (f);
	return;
}
void Log::load (const char *fname) {STACKTRACE
	PACKFILE *f;
	char buffy[1024];
	int i, oi, j, mj;
	f = pack_fopen ( fname, F_READ_PACKED);
	if (!f) throw("Log::load() failed");
	mj = pack_igetl ( f);
	oi = -1;
	for (j = 0; j < mj; j += 1) {
		f = pack_fopen_chunk(f, 0);
		i = pack_igetl ( f);
		if (i <= oi) { tw_error("invalid log file"); }
		if (i >= log_num) expand_logs(i+1);
		int l = pack_igetl ( f);
		while ( l > 0) {
			int tl = 1024;
			if (tl > l) tl = l;
			int k = pack_fread(buffy, tl, f);
			if (k == 0) { tw_error("invalid log file"); }
			l -= k;
			_log(i, buffy, tl);
		}
		f = pack_fclose_chunk(f);
	}
	pack_fclose (f);
	return;
}
void Log::expand_logs(int num_channels) {
	STACKTRACE;
	int old_log_num = log_num;
	if (num_channels <= log_num) { tw_error ("Log::expand_logs - shrinking logs?"); }
	log_num = num_channels;
	log_data = (unsigned char**) realloc(log_data, sizeof(char*) * log_num);
	log_size    = (int*) realloc(log_size, sizeof(int) * log_num);
	log_pos     = (int*) realloc(log_pos,  sizeof(int) * log_num);
	log_len     = (int*) realloc(log_len,  sizeof(int) * log_num);
	log_dir     = (char*) realloc(log_dir,  sizeof(char) * log_num);
	for (int i = old_log_num; i < log_num; i += 1) {
		log_data[i] = NULL;
		log_size[i] = 0;
		log_len[i] = 0;
		log_pos[i] = 0;
		log_dir[i] = default_direction;
	}
	return;
}
int Log::ready(int channel) {
	if (channel < 0) { tw_error ("log_ready - negative channel!"); }
	if (channel >= log_num) return 0;
	return log_len[channel] - log_pos[channel];
}
int Log::file_ready(const char *fname, void **location) {
	STACKTRACE;
	if (log_num <= channel_file_data) return -1;
	int i = 0, j = 0;
	while (i < log_len[channel_file_names]) {
		if (strcmp((char*) log_data[channel_file_names] + i, fname)) {
			i += strlen((char*) log_data[channel_file_names] + i) + 1;
//			j += intel_ordering(*((int*)(log_data[channel_file_names] + i)));
			int k;
			memcpy(&k, (log_data[channel_file_names] + i), sizeof(int));
			j += intel_ordering(k);
			i += sizeof(int);
		}
		else {
			i += strlen((char*) log_data[channel_file_names] + i) + 1;
			int k;
			memcpy(&k, (log_data[channel_file_names] + i), sizeof(int));
			k = intel_ordering(k);

			if (j+k > log_len[channel_file_data]) { tw_error ("Log::file_ready - uh, that's bad"); }
			if (location) *location = log_data[channel_file_data] + j;
			return k;
		}
	}
	return -1;
}
void Log::log_file(const char *fname) {STACKTRACE
	void *loc;
	if (!(log_dir[channel_file_data] & direction_read)) {
		set_config_file(fname);
		return;
	}
	int len = file_ready(fname, &loc);
	if (len >= 0) {
		set_config_data((char*)loc, len);
		return;
	}
	if (!(log_dir[channel_file_data] & direction_write)) {
		tw_error("Log::log_file - file logs read only, \"%s\" not found", fname);
	}
	if (log_num <= channel_file_data) {
		expand_logs(channel_file_data + 1);
	}
	char buffy[2048];
	PACKFILE *f;
	int i, j = 0;
	f = pack_fopen(fname, F_READ);
	if (!f) { tw_error("tw_log_file - bad file name %s", fname); }
	while (1) {
		i = pack_fread(buffy, 1024, f);
		if (i > 0) {
			_log(channel_file_data, buffy, i);
			j += i;
		}
		else break;
	}
	sprintf(buffy, "%s", fname);
	j = intel_ordering(j);
	memcpy(&buffy[strlen(buffy)+1], &j, sizeof(int));
	_log (channel_file_names, buffy, strlen(buffy)+5);
	len = file_ready(fname, &loc);
	set_config_data((char*)loc, len);
	return;
}

void Log::deinit() {STACKTRACE
	return;
}

bool Log::buffer ( int channel, void *data, int size ) {STACKTRACE
	char zeros[128];
	if (!size) return false;//return true?  error?  
	if (channel < 0) { tw_error ("Log::log - negative channel!"); }
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	if (data == NULL) {
		data = zeros;
		if (size > 128) { tw_error("Log::buffer - overflow"); }
		memset(zeros, 0, size);
	}
	if (log_dir[channel] & direction_write) {
		_log ( channel, data, size);
//		if (!(log_dir[channel] & direction_immediate)) send_packet();
		return true;
	}
//	if (log_dir[channel] & direction_read) _unlog ( channel, data, size);
	return false;
}

bool Log::unbuffer ( int channel, void *data, int size ) {STACKTRACE
	char zeros[128];
	if (!size) return false;//return true?  error?  
	if (channel < 0) { tw_error ("Log::unbuffer - negative channel!"); }
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	if (data == NULL) {
		data = zeros;
		if (size > 128) { tw_error("Log::buffer - overflow"); }
	}

//	if (log_dir[channel] & direction_write) _log ( channel, data, size);
	if (log_dir[channel] & direction_read) {
		_unlog ( channel, data, size);
		return true;
	}
	return false;
}

void Log::flush() {
	return;
}

bool Log::listen() {
	return false;
}

void Log::reset() {
	int i;
	for (i = 0 ; i < this->log_num; i += 1) {
		log_pos[i] = 0;
		}
	return;
}



void PlaybackLog::init() {STACKTRACE
	Log::init();
	playback = true;
	default_direction = Log::direction_read;
	Log::set_direction(Game::channel_playback, Log::direction_read | Log::direction_write | Log::direction_forget);
	Log::set_direction(Game::channel_playback + Game::_channel_buffered, Log::direction_read | Log::direction_write | Log::direction_forget);
	return;
}

void PlaybackLog::set_direction (int channel, char direction) {
	STACKTRACE;
	tw_error("set_direction - your not supposed to do that in a demo playback!");
	return;
}

void PlaybackLog::set_all_directions( char direction ) {
	STACKTRACE;
	tw_error("set_all_directions - your not supposed to do that in a demo playback!");
	return;
}




static enum share_types {TYPE_CHAR, TYPE_SHORT, TYPE_INT};
static const int max_share = 512;
static int Nshare = 0;
static int share_channel[max_share];
static int share_size[max_share];
static void *share_address[max_share];
static int share_type[max_share];
static int share_num[max_share];



void share_intel_order(int n, int i)
{
	switch(share_type[n])
	{
	case TYPE_SHORT:
		{
			short int &x = *((short int*) (share_address[n]) + i);
			x = intel_ordering_short(x);
		}
		break;

	case TYPE_INT:
		{
			int &x = *((int*) (share_address[n]) + i);
			x = intel_ordering(x);
		}
		break;
	}
}

void share_buffer(int channel, void *value, int num, int size, share_types st)
{
	share_channel[Nshare] = channel;
	share_address[Nshare] = value;

	share_num[Nshare] = num;
	share_size[Nshare] = size;

	share_type[Nshare] = st;

	// change bit-order of each element in an array
	int i;
	for ( i = 0; i < share_num[Nshare]; ++i )
		share_intel_order(Nshare, i);

	// buffer an array
	game->log->buffer(share_channel[Nshare],
							share_address[Nshare],
							share_num[Nshare] * share_size[Nshare]);

	// restore bit-order of each element in an array
	for ( i = 0; i < share_num[Nshare]; ++i )
		share_intel_order(Nshare, i);

	//game->log->flush();

	++Nshare;
}

/** \brief Share an integer value at a memory location across the network. This is pushed
on a stack, and has to be read afterwards. Note that no other networking operation should
occur between the share and share_update(). It uses the global game->log. NOTE, that there
is no "intel_buffering" done, so it's probably a problem between different machine types.
*/

void share(int channel, int *value, int num)
{
	share_buffer(channel, value, num, sizeof(int), TYPE_INT);
}


void share(int channel, short *value, int num)
{
	share_buffer(channel, value, num, sizeof(short), TYPE_SHORT);
}


void share(int channel, char *value, int num)
{
	share_buffer(channel, value, num, sizeof(char), TYPE_CHAR);
}




/** \brief Retrieves the values that were sent for sharing, and writes the values
to the proper memory locations.
*/
void share_update()
{
	// do it here, cause why would you want to wait for each little packet till it
	// can be sent onto the net ?
	game->log->flush();

	// superfluous, cause this is already used inside the unbuffer routine.
	//game->log->listen();


	int n;
	for ( n = 0; n < Nshare; ++n )
	{
		// unbuffer an array
		game->log->unbuffer(share_channel[n], share_address[n], share_size[n] * share_num[n]);

		// sort out bit-ordering for all values in the array
		int i;
		for ( i = 0; i < share_num[n]; ++i )
			share_intel_order(n, i);
	}

	Nshare = 0;
}
