/* $Id$ */ 
#ifndef __MNET1_H__
#define __MNET1_H__

#include "mmain.h"
#include "../util/net_tcp.h"
#include "mlog.h"
#include "mship.h"


class GameEvent {
public:
	short int size;
	short int type;
	GameEvent() {
		size = sizeof(this); 
		type = Game::event_invalid;
	}
	void *operator new (size_t size) {return malloc(size);}
};
class GameEventChangeLag : public GameEvent {
public:
	short int old_lag;
	short int new_lag;
	GameEventChangeLag( int lag_frames ) {
		size = sizeof(this);
		type = Game::event_change_lag;
		old_lag = game->lag_frames;
		new_lag = lag_frames;
	}
	void execute ( int source );
};
class GameEventMessage : public GameEvent {
public:
	enum { max_message_length = 150 };
	char message[max_message_length];
	GameEventMessage( const char *text ) ;
	void execute ( int source );
};

/*
class LagHandler {
	unsigned char lag;  //a copy of lag_frames
	unsigned char size; //bytes per frame
	char channel;       //channel to transmit on
	void *data;         //serialized
//	void log ( void *data, 
};*/


const int max_connections = 100;
// note, that I'll assume that (channel/4) equals the connection number.

class NetLog : public Log { //Logging system, useful for networking & demo recording/playback
	protected:

	int *log_transmitted;      //the number of bytes transmitted in each channel
	void expand_logs(int num_channels) ; //intializes these extensions to the logging
	void check_bufsize(int size);		// checks if the buffer isn't too small, and increases its size if needed
	unsigned char *buffy;	//a buffer for sending and recieving packets
	int buffy_size;			// current size of the buffer.
	//int buffy_num;				// how many prepared data are waiting in the buffer
//	void handle_code(unsigned int code) ;
//	void send_code(unsigned int code) ;
	enum code {
		NET1_CODE_QUIT    = 0x80000001,
		NET1_CODE_PAUSE   = 0x80000002,
		NET1_CODE_UNPAUSE = 0x80000003,
		NET1_CODE_INCREASE_LAG = 0x80000004,
		NET1_CODE_DECREASE_LAG = 0x80000005,
		NET1_CODE_MESSAGE = 0x90000000
		};
//	void send_message(char *string) ;

	void prepare_packet();
		
	void send_packets(); //sends a packet (copies of it) to all outgoing connections
	void send_packet_block(int conn); //sends a (copy of a) packet to a particular connection

	public:

	bool need_to_transmit;
	int num_connections;
	NetTCP net[max_connections];       // the TCP network connection to the opponent player
	bool net_status[max_connections];

	int conn(int channel) { return channel / 4; };

	/*
	enum {
		direction_immediate = 16
		};
		*/

	void recv_packet(int conn); //receives a packet

	int remote_time[max_connections]; //used in calculating ping
	int ping[max_connections];        //the most recently measured ping

	virtual void init();
	virtual void deinit();
	virtual ~NetLog();
	virtual void _log(int channel, const void *data, int size);   //used for recording data.  this version may transmit it over the network
	virtual void _unlog(int channel, void *data, int size); //used for playing back data.  this version may recieve it over the network
	void log_file(const char *fname) ;
	virtual int ready(int channel);

	virtual void flush_block() ;
	virtual void flush_noblock();
	virtual bool listen();


	bool add_listen(int port);
	bool add_connect(const char *address, int port);

	char *get_address(int n);

	void optimize4latency();

	virtual void use_idle(int time);
	
	void recv_noblock();
	bool ready2send();
	void send_packet_noblock(int conn);

	virtual void force_update();

	virtual void reset();

	void rem_conn(int conn);

	};

#endif // __MNET1_H__
