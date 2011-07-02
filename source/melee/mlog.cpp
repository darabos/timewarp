/* $Id$ */ 
#include <stdio.h>
#include <string.h>
#include <allegro.h>

#include "../melee.h"
#include "../scp.h"

#include "mlog.h"
#include "mgame.h"
#include "../util/net_tcp.h"
#include "../melee/mnet1.h"
#include "../melee/mview.h"

int channel_current = -1;
int event_player_current = 0;

int channel_file_names = 0;		// ISN'T USED ...
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


void Log::init() { 
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



void Log::deinit()
{
	 

	if (log_data)
	{
		for (int i = 0; i < log_num; i += 1)
			free(log_data[i]);
	}

	if (log_len) free(log_len);
	log_len = 0;

	if (log_size) free(log_size);
	log_size = 0;

	if (log_pos) free(log_pos);
	log_pos = 0;

	if (log_data) free(log_data);
	log_data = 0;

	if (log_dir) free(log_dir);
	log_dir = 0;

	log_num = 0;

	return;
}


Log::~Log()
{
	 

	deinit();
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
	 
	if (channel < 0) { throw("set_direction - channel < 0"); }
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	log_dir[channel] = direction;
	return;
}

char Log::get_direction ( int channel ) {
	 
	if (channel < 0) {throw("get_direction - channel < 0");}
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	return (log_dir[channel]);
}

void Log::set_all_directions ( char direction ) { 
	default_direction = direction;
	int i;
	for (i = 0; i < log_num; i += 1) {
		log_dir[i] = direction;
	}
	return;
}


void Log::log ( int channel, void *data, int size)
{
	 

	if (channel == 0)
		throw("attempting to write to a forbidden channel");

	if (!size) return;
	if (channel < 0) {throw ("Log::log - negative channel!");}
	if (channel >= log_num) {
		expand_logs(channel+1);
	}

	if (!log_dir[channel]) _null_log(channel, data, size);
	if ((log_dir[channel] & direction_write) && !log_write_disable) _log ( channel, data, size);
	if ((log_dir[channel] & direction_read ) && !log_read_disable && !log_fake ) _unlog ( channel, data, size);
	return;
}


void Log::_null_log(int channel, void *data, int size)
{
	memset(data, 0, size);	// the channel doesn't matter in this case.
	// just set requested data to a predictable size of 0.
}

void Log::_log(int channel, const void *data, int size)
{
	 

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
	 
	if (log_len[channel] < log_pos[channel] + size) {throw ("Game::_unlog - went past end (%d+%d/%d on %d)", log_pos[channel], size, log_len[channel], channel);}
	memcpy(data, log_data[channel]+log_pos[channel], size);
	log_pos[channel] += size;
	return;
}

void Log::save (const char *fname) { 
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
void Log::load (const char *fname) { 
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
		if (i <= oi) { throw("invalid log file"); }
		if (i >= log_num) expand_logs(i+1);
		int l = pack_igetl ( f);
		while ( l > 0) {
			int tl = 1024;
			if (tl > l) tl = l;
			int k = pack_fread(buffy, tl, f);
			if (k == 0) { throw("invalid log file"); }
			l -= k;
			_log(i, buffy, tl);
		}
		f = pack_fclose_chunk(f);
	}
	pack_fclose (f);
	return;
}
void Log::expand_logs(int num_channels) {
	 
	int old_log_num = log_num;
	if (num_channels <= log_num) { throw ("Log::expand_logs - shrinking logs?"); }
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
	if (channel < 0) { throw ("log_ready - negative channel!"); }
	if (channel >= log_num) return 0;
	return log_len[channel] - log_pos[channel];
}




bool Log::buffer ( int channel, void *data, int size ) { 
	char zeros[128];
	if (!size) return false;//return true?  error?  
	if (channel < 0) { throw ("Log::log - negative channel!"); }
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	if (data == NULL) {
		data = zeros;
		if (size > 128) { throw("Log::buffer - overflow"); }
		memset(zeros, 0, size);
	}
	if ((log_dir[channel] & direction_write) && (!log_write_disable) ) {
		_log ( channel, data, size);
//		if (!(log_dir[channel] & direction_immediate)) send_packet();
		return true;
	}
//	if (log_dir[channel] & direction_read) _unlog ( channel, data, size);
	return false;
}

bool Log::unbuffer ( int channel, void *data, int size ) { 
	char zeros[128];
	if (!size) return false;//return true?  error?  
	if (channel < 0) { throw ("Log::unbuffer - negative channel!"); }
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	if (data == NULL) {
		data = zeros;
		if (size > 128) { throw("Log::buffer - overflow"); }
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

/*
Well, there can be a problem here, if you reset a log, while one of the remote
computers has already passed the reset() line, and has already sent new data to
this computer. In that case, there's still data to be read, and the reset should
not delete those data - those data belong to the program-part that comes AFTER the
reset call, so it's ok to keep them in store.
*/
void Log::reset()
{
	int i;
	for (i = 0 ; i < log_num; i += 1)
	{
		// added GEO:
		if (log_len[i] - log_pos[i] > 0)
		{
	//		throw("Resetting a non-empty log  channel[%i] pos[%i] len[%i] !!",
	//			i, log_pos[i], log_len[i]);
			// actually, the line above doesn't adress a real error, but it can be
			// caused by a remote computer, who's executing actions slightly earlier than
			// this computer, and who could've already sent some data to this computer,
			// which this computer has happily accepted. Since these data belong to
			// actions *after* the reset point, you've to keep them for later use,

			int k = log_len[i] - log_pos[i];

			memcpy(log_data[i], log_data[i] + log_pos[i], k);

			log_len[i] = k;	// keep the lingering data, since those belong to subsequent actions,
			//originating from too-early actions by remote computers in the game
			log_pos[i] = 0;
		} else {

			log_len[i] = 0;
			log_pos[i] = 0;
		}
	}
	
	return;
}


/** completely clear a log, also get rid of unhandled data */
void Log::clear()
{
	int i;
	for (i = 0 ; i < log_num; i += 1)
	{
		log_len[i] = 0;
		log_pos[i] = 0;
	}
	
}




void PlaybackLog::init() { 
	Log::init();
	playback = true;
	default_direction = Log::direction_read;
	Log::set_direction(channel_playback, Log::direction_read | Log::direction_write | Log::direction_forget);
	Log::set_direction(channel_playback + _channel_buffered, Log::direction_read | Log::direction_write | Log::direction_forget);
	return;
}

void PlaybackLog::set_direction (int channel, char direction) {
	 
	throw("set_direction - your not supposed to do that in a demo playback!");
	return;
}

void PlaybackLog::set_all_directions( char direction ) {
	 
	throw("set_all_directions - your not supposed to do that in a demo playback!");
	return;
}




enum share_types {TYPE_CHAR, TYPE_SHORT, TYPE_INT, TYPE_DOUBLE};
static const int max_share = 512;
static int Nshare = 0;
static int share_channel[max_share];
static int share_size[max_share];
static void *share_address[max_share];
static int share_type[max_share];
static int share_num[max_share];


int get_share_num()
{
	return Nshare;
}

void reset_share()
{
	Nshare = 0;
}


void share_intel_order(int n)
{
	int i;
	for ( i = 0; i < share_num[n]; ++i )
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
}


static int chann(int iplayer)
{
	if (iplayer == -1)
		return channel_init;

	else
	{
		if (!player[iplayer])
			throw("Player does not exist - no channel available");
		return player[iplayer]->channel;
	}
}

void share_buffer(int player, void *value, int num, int size, share_types st)
{
	if (chann(player) == channel_none)
		return;
	if (player > num_players)
		throw("player [%i] exceeds max allowed value [%i] -- perhaps you use share(channel) instead of share(player)", player, num_players);


	share_channel[Nshare] = chann(player);
	share_address[Nshare] = value;

	share_num[Nshare] = num;
	share_size[Nshare] = size;

	share_type[Nshare] = st;

	// change bit-order of each element in an array
	share_intel_order(Nshare);

	/*
	// check some stuff: (only works on network cause of the cast)
	int ch = share_channel[Nshare];
	int log_len1 = glog->log_len[ch];
	int log_transm1 = ((NetLog*)glog)->log_transmitted[ch];
	*/


	// buffer an array
	glog->buffer(share_channel[Nshare],
							share_address[Nshare],
							share_num[Nshare] * share_size[Nshare]);

	// restore bit-order of each element in an array
	share_intel_order(Nshare);

	//game->log->flush();

	/*
	// check some stuff: (only works on network cause of the cast)
	char dir = glog->get_direction(share_channel[Nshare]);
	int bwrite = dir & Log::direction_write;
	int bread = dir & Log::direction_read;
	int bimm = dir & Log::direction_immediate;
	int log_len = glog->log_len[ch];
	int log_transm = ((NetLog*)glog)->log_transmitted[ch];

	message.print(1500, 14, "SHAREBUFFER: ch[%i] write[%i] read[%i] imm[%i]  len[%i->%i] transm[%i->%i]",
					ch, bwrite, bread, bimm, log_len1, log_len, log_transm1, log_transm);
	message.animate(0);
	*/

	++Nshare;

	//message.print(1500, 15, "Nshare[%i] size[%i]", Nshare, share_num[Nshare-1] * share_size[Nshare-1]);
	//message.animate(0);
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


void share(int player, unsigned int *value, int num)
{
	share_buffer(player, value, num, sizeof(unsigned int), TYPE_INT);
}


void share(int player, short *value, int num)
{
	share_buffer(player, value, num, sizeof(short), TYPE_SHORT);
}


void share(int player, char *value, int num)
{
	share_buffer(player, value, num, sizeof(char), TYPE_CHAR);
}

void share(int player, double *value, int num)
{
	share_buffer(player, value, num, sizeof(double), TYPE_DOUBLE);
}







/** \brief Retrieves the values that were sent for sharing, and writes the values
to the proper memory locations.
*/
#include "mmain.h"
void share_update()
{
	int n;

	/*
	// check some stuff...
	for ( n = 0; n < Nshare; ++n )
	{
		int ch = share_channel[n];
		char dir = glog->get_direction(share_channel[n]);
		int bwrite = dir & Log::direction_write;
		int bread = dir & Log::direction_read;
		int bimm = dir & Log::direction_immediate;
	
		message.print(1500, 14, "SHAREUPDATE: ch[%i] write[%i] read[%i] imm[%i]", ch, bwrite, bread, bimm);
		message.animate(0);
	}
	*/

	// test the network, whether the log channels are still ok.
	// by using a call to log_test();
	// but NOT here, cause at this moment, you can have mixing of data. You've to
	// wait till the (delayed) buffered data (from calls to share()) are received,
	// since those are already on their way on the net..
	// you've to test at the end of this subroutine.

	glog->force_update();
	glog->flush_block();	// this only blocks, if need_to_tranmit == true.
	// so, for that I call force_update()

	// superfluous, cause this is already used inside the unbuffer routine.
	//game->log->listen();


	for ( n = 0; n < Nshare; ++n )
	{
		// unbuffer an array
		glog->unbuffer(share_channel[n], share_address[n], share_size[n] * share_num[n]);

		// sort out bit-ordering for all values in the array
		share_intel_order(n);
	}

	Nshare = 0;

	// test the network, whether the log channels are still ok.
	// it's safe to do here, cause all lingering requests should've
	// been updated by now !!
//	log_test();
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
	 

	int chold = channel_current;
	channel_current = channel_file_data;

	if (log_read_disable || log_write_disable || log_fake)
		throw("Faking sharing a file ? Unlikely !");

	if (writeable() && !(log_dir[channel_current] & direction_immediate))
		throw("You should use an immediate connection to share file-data");

	int L = 0;
	if (writeable())
		L = file_size(fname);

	char *buffy = create_buffer(&L);

	if (writeable())
	{	
		PACKFILE *f;
		
		int i;
		f = pack_fopen(fname, F_READ);
		if (!f) { throw("tw_log_file - bad file name %s", fname); }
		
		i = pack_fread(buffy, L, f);
		if ( i != L ) { throw("tw_log_file - bad filesize"); };
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
		throw("The file channel should be empty. Contains %i chars", log_size_ch(channel_current));
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



void rand_resync()
{
	
	//int i1, i2;

	//i1 = rand();
	//i2 = rand();

	//share(-1, &i1);
	//share(-1, &i2);
	//share_update();

	//rng.seed(i1);
	//rng.seed_more(i2);

	seed_ohmy();

}
