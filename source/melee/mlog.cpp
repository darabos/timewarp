#include <stdio.h>
#include <string.h>
#include <allegro.h>

#include "../melee.h"
#include "../scp.h"

#include "mlog.h"
#include "mgame.h"
#include "../util/net_tcp.h"
#include "../melee/mview.h"

int channel_current = -1;

int channel_file_names = 0;
int channel_file_data = 1; //direction for file channels must be the same


////////////////////////////////////////////////////////////////////////
//				Logging stuff
////////////////////////////////////////////////////////////////////////


// if set to fake, it overrides all other read-settings to "false", to allow buffering
// of fake data, without having to make this explicit in higher level code.
static bool log_fake = false;

void log_set_fake()
{
	log_fake = true;
}

void log_set_nofake()
{
	log_fake = false;
}

// to initialize a lag-buffer, you've to first push fake data, without reading operations,
// i.e. a predictable (and therefore synched), pushing of pre-defined (hard-coded) values.
// For this, you need stuff like this (and also to push/pop sequences of values to the buffer)

static bool log_read_disable = false;	// turn off most of the time, except when building the lag buffer (then you just write into the buffer!)
static bool log_write_disable = false;
bool log_synched = true;

void log_set_default()
{
	log_read_disable = false;
	log_write_disable = false;
}

void log_set_writeonly()
{
	log_read_disable = true;	// disable read (always)
	log_write_disable = false;
}

void log_set_readonly()
{
	log_read_disable = log_fake;	// enable read, but only if it's not in fake-mode
	log_write_disable = true;		// disable write
}


// check if the logging mode is currently read-only. This is useful for events, which
// should only "act" if you're in receiving mode.
bool log_readonly()
{
	return (log_read_disable == false) && (log_write_disable == true);
}

bool log_writeonly()
{
	return (log_read_disable == true) && (log_write_disable == false);
}

bool log_default()
{
	return (log_read_disable == false) && (log_write_disable == false);
}


// check current channel settings
bool log_writable()
{
	return (glog->writeable());
}


bool log_readable()
{
	return (glog->readable());
}

// define different "modes" of writability or readability
void log_resetmode()
{
	log_set_default();
	log_synched = true;
}

// default --> writeonly --> readonly
bool log_nextmode()
{
	if (log_default())
	{
		log_set_writeonly();
		log_synched = false;	// written data are true-time, hence, not synched
		return true;
	}
	else if (log_writeonly())
	{
		log_set_readonly();
		if (!log_fake)
		{
			log_synched = true;		// received data are game-time, hence, synched
			return true;
		} else {
			log_synched = false;
			return false;
		}

	}

	else
		return false;
}




bool log_show_data = false;	// for inspecting values


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

void Log::set_r(int ch)
{
	set_direction(ch , direction_read);
	set_direction(ch + _channel_buffered, direction_read);
}

void Log::set_rw(int ch)
{
	set_direction(ch, direction_write | direction_read | direction_immediate);
	set_direction(ch + _channel_buffered, direction_write | direction_read);
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

	if ((log_dir[channel] & direction_write) && !log_write_disable) _log ( channel, data, size);
	if ((log_dir[channel] & direction_read ) && !log_read_disable && !log_fake ) _unlog ( channel, data, size);
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
	if ((log_dir[channel] & direction_write) && !log_write_disable) {
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
	if ((log_dir[channel] & direction_read) && !log_read_disable && !log_fake) {
		_unlog ( channel, data, size);
		return true;
	}
	return false;
}

void Log::flush_block() {
	return;
}

void Log::flush_noblock() {
	return;
}

bool Log::listen() {
	return false;
}

void Log::use_idle(int time)
{
	// nothing by default.
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
	Log::set_direction(channel_playback, Log::direction_read | Log::direction_write | Log::direction_forget);
	Log::set_direction(channel_playback + _channel_buffered, Log::direction_read | Log::direction_write | Log::direction_forget);
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




enum share_types {TYPE_CHAR, TYPE_SHORT, TYPE_INT};
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


static int chann(int player)
{
	if (player == -1)
		return channel_init;

	else
		return channel_player[player];
}

void share_buffer(int player, void *value, int num, int size, share_types st)
{
	if (chann(player) == channel_none)
		return;

	share_channel[Nshare] = chann(player);
	share_address[Nshare] = value;

	share_num[Nshare] = num;
	share_size[Nshare] = size;

	share_type[Nshare] = st;

	// change bit-order of each element in an array
	int i;
	for ( i = 0; i < share_num[Nshare]; ++i )
		share_intel_order(Nshare, i);

	// buffer an array
	glog->buffer(share_channel[Nshare],
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

void share(int player, int *value, int num)
{
	share_buffer(player, value, num, sizeof(int), TYPE_INT);
}


void share(int player, short *value, int num)
{
	share_buffer(player, value, num, sizeof(short), TYPE_SHORT);
}


void share(int player, char *value, int num)
{
	share_buffer(player, value, num, sizeof(char), TYPE_CHAR);
}







/** \brief Retrieves the values that were sent for sharing, and writes the values
to the proper memory locations.
*/
void share_update()
{
	// do it here, cause why would you want to wait for each little packet till it
	// can be sent onto the net ?

	
//	glog->need_to_transmit = true;
	glog->flush_block();

	// superfluous, cause this is already used inside the unbuffer routine.
	//game->log->listen();


	int n;
	for ( n = 0; n < Nshare; ++n )
	{
		// unbuffer an array
		glog->unbuffer(share_channel[n], share_address[n], share_size[n] * share_num[n]);

		// sort out bit-ordering for all values in the array
		int i;
		for ( i = 0; i < share_num[n]; ++i )
			share_intel_order(n, i);
	}

	Nshare = 0;
}





bool Log::writeable(int ch)
{
	if (ch < 0)
		return false;

	return log_dir[ch] & direction_write;
}


bool Log::readable(int ch)
{
	if (ch < 0)
		return false;

	return log_dir[ch] & direction_read;
}




void Log::lint(int *val, int ch)
{
	log(ch, val, sizeof(int));
}

void Log::ldata(char *data, int N, int ch)
{
	log(ch, data, N);
}

char *Log::create_buffer(int *size, int ch)
{
	lint(size, ch);
	return new char [*size];
}


void Log::log_file(const char *fname)
{
	STACKTRACE;

	int chold = channel_current;
	channel_current = channel_file_data;

	if (log_read_disable || log_write_disable || log_fake)
		tw_error("Faking sharing a file ? Unlikely !");

	if (writeable() && !(log_dir[channel_current] & direction_immediate))
		tw_error("You should use an immediate connection to share file-data");

	int L = 0;
	if (writeable())
		L = file_size(fname);

	char *buffy = create_buffer(&L);

	if (writeable())
	{	
		PACKFILE *f;
		
		int i, j = 0;
		f = pack_fopen(fname, F_READ);
		if (!f) { tw_error("tw_log_file - bad file name %s", fname); }
		
		i = pack_fread(buffy, L, f);
		if ( i != L ) { tw_error("tw_log_file - bad filesize"); };
	}

	ldata(buffy, L, sizeof(char));	// woops, I used sizeof(int) by mistake --> caused some unpred.behav.

	set_config_data(buffy, L);

	/*
	// error check:
	int i;
	int sum = 0;
	for ( i = 0; i < L; ++i )
		sum += buffy[i];

	message.print(1500, 14, "L = %i  sum = %i", L, sum);
	message.animate(0);
	//*/

	delete buffy;


	channel_current = chold;

	/*
	// check if the file-share buffer isn't empty - it should be empty !!
	// but this only makes sense for the host, cause remotes who are on the receiving end,
	// can receive >1 file due to the lag, if the game is paused.
	if (log_size_ch(channel_current) != 0)
		tw_error("The file channel should be empty. Contains %i chars", log_size_ch(channel_current));
	so ... in principle this test makes sense, but only in places where it's safe to test. Like,
	after all ships have been initialized.
	*/

	return;
}



int log_size_ch(int channel)
{
	if (glog)
		return glog->log_len[channel] - glog->log_pos[channel];
	else
		return 0;
}
