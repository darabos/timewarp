/* $Id$ */ 
#ifndef __MLOG_H__
#define __MLOG_H__

extern int channel_file_names;
extern int channel_file_data; //direction for file channels must be the same

void log_set_fake();
void log_set_nofake();


void log_set_default();

// disables write operations
void log_set_readonly();

// disables read operations
void log_set_writeonly();

// check current read/write override settings
bool log_readonly();
bool log_writeonly();
bool log_default();

extern bool log_synched;	// use in combination with "log_modes"
void log_resetmode();
bool log_nextmode();


// check current channel settings
bool log_writable();
bool log_readable();

extern int channel_current;

int log_size_ch(int channel);


extern bool log_show_data;	// for inspecting values


class Log { //Logging system, usefull for networking & demo recording/playback
	protected:
	public:
	int log_num;   //number of channels in use (0 to log_num-1 in use)
	unsigned char **log_data;
	char default_direction;
	char *log_dir; //the direction (sending/recieving/etc) of each channel
	int *log_size; //size of memory allocated for each channel (bytes)
	int *log_len;  //length of channel (bytes)
	int *log_pos;  //position of reading from channel (bytes)
	virtual void expand_logs(int num_channels) ;
	virtual void _log   (int channel, const void *data, int size) ;
	virtual void _unlog (int channel, void *data, int size) ;
//	static int log_pos2[256]; //position of reading from channel for network purposes (bytes)
//	static int log_dir[256];  //direction of channel (none, self->other, other->self, self->self)
//	static int log_dir_files;    //direction of channel (none, self->other, other->self, self->self)
//	static int log_dir_messages; //direction of channel (none, self->other, other->self, self->self)

	public:
	int type;
	enum {
		log_normal = 0, log_net = 1
		};
	bool playback;
	enum {
		direction_write = 1, direction_read = 2, direction_forget = 4, direction_immediate = 16
		};
	virtual void log (int channel, void *data, int size) ;
	virtual void init();
	virtual void deinit();

	virtual void set_direction (int channel, char direction);
	virtual char get_direction (int channel);
	void set_all_directions (char direction);

	virtual void set_r(int ch);
	virtual void set_rw(int ch);

	virtual ~Log();
	virtual void log_file(const char *fname);
//	virtual void *log_file_data (const char *fname, int *size);
	virtual int ready(int channel);
	//file_ready returns -1 if a file is not ready, otherwise it is the length of the file.  if location is not NULL, it sets *location to pointing at the file data.  
//	virtual int file_ready(const char *fname, void **location = NULL);

	virtual void save (const char *fname);
	virtual void load (const char *fname);

	virtual bool buffer (int channel, void *data, int size );
	virtual bool unbuffer (int channel, void *data, int size );

	virtual void flush_block();
	virtual void flush_noblock();
	virtual bool listen();
	virtual void reset();

	virtual void use_idle(int time);	// is called in the games idle() function

	bool writeable(int ch = channel_current);
	bool readable(int ch = channel_current);

	void lint(int *val, int ch = channel_current);
	void ldata(char *data, int N, int ch = channel_current);
	char *Log::create_buffer(int *size, int ch = channel_current);

	virtual void force_update() {};

	void _null_log(int channel, void *data, int size);
};

class PlaybackLog : public Log { //Logging system, usefull for networking & demo recording/playback
	virtual void init();
	virtual void set_direction (int channel, char direction);
	virtual void set_all_directions (char direction);
	};


/** \brief Share memory values accross player channels.
*/
void share(int netnum, int *value, int num = 1);
void share(int netnum, short *value, int num = 1);
void share(int netnum, char *value, int num = 1);
void share(int player, double *value, int num = 1);
void share_update();

/** \brief Helper to synch random numbers between networked computers - this is
useful in case that the host initialization requires random numbers, which aren't
synched ... although you could also use "rand()" in that case, which doesn't affect
the random numbers but well...
*/
void rand_resync();

#endif  // __MLOG_H__
