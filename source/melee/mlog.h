#ifndef __MLOG_H__
#define __MLOG_H__


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
	enum {
		channel_file_names = 0, channel_file_data = 1 //direction for file channels must be the same
		};
	public:
	int type;
	enum {
		log_normal = 0, log_net1server = 2, log_net1client = 3
		};
	bool playback;
	enum {
		direction_write = 1, direction_read = 2, direction_forget = 4
		};
	virtual void log (int channel, void *data, int size) ;
	virtual void init();
	virtual void deinit();

	virtual void set_direction (int channel, char direction);
	virtual char get_direction (int channel);
	void set_all_directions (char direction);

	virtual ~Log();
	virtual void log_file(const char *fname);
//	virtual void *log_file_data (const char *fname, int *size);
	virtual int ready(int channel);
	//file_ready returns -1 if a file is not ready, otherwise it is the length of the file.  if location is not NULL, it sets *location to pointing at the file data.  
	virtual int file_ready(const char *fname, void **location = NULL);

	virtual void save (const char *fname);
	virtual void load (const char *fname);

	virtual bool buffer (int channel, void *data, int size );
	virtual bool unbuffer (int channel, void *data, int size );

	virtual void flush();
	virtual bool listen();
	virtual void reset();
	};

class PlaybackLog : public Log { //Logging system, usefull for networking & demo recording/playback
	virtual void init();
	virtual void set_direction (int channel, char direction);
	virtual void set_all_directions (char direction);
	};

void share(int channel, int *value);
void share_update();


#endif  // __MLOG_H__
