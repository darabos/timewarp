#ifndef _MMAIN_H
#include "mmain.h"
#endif
#ifndef _NET_TCP_H
#include "../util/net_tcp.h"
#endif
#ifndef _MLOG_H
#include "mlog.h"
#endif
#ifndef _MSHIP_H
#include "mship.h"
#endif



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


class NetLog : public Log { //Logging system, usefull for networking & demo recording/playback
	protected:

	int *log_transmitted;      //the number of bytes transmitted in each channel
	void expand_logs(int num_channels) ; //intializes these extensions to the logging
	unsigned char buffy[4096]; //a buffer for sending and recieving packets
	void handle_code(unsigned int code) ;
	void send_code(unsigned int code) ;
	enum code {
		NET1_CODE_QUIT    = 0x80000001,
		NET1_CODE_PAUSE   = 0x80000002,
		NET1_CODE_UNPAUSE = 0x80000003,
		NET1_CODE_INCREASE_LAG = 0x80000004,
		NET1_CODE_DECREASE_LAG = 0x80000005,
		NET1_CODE_MESSAGE = 0x90000000
		};
	void send_message(char *string) ;

	void send_packet(); //sends a packet

	public:

	bool need_to_transmit;
	NetTCP net;       // the TCP network connection to the opponent player

	enum {
		direction_immediate = 16
		};

	void recv_packet(); //recieves a packet

	int remote_time; //used in calculating ping
	int ping;            //the most recently measured ping

	virtual void init();
	virtual void deinit();
	virtual ~NetLog();
	virtual void _log(int channel, const void *data, int size);   //used for recording data.  this version may transmit it over the network
	virtual void _unlog(int channel, void *data, int size); //used for playing back data.  this version may recieve it over the network
	void log_file(const char *fname) ;
	virtual int ready(int channel);

	virtual void flush() ;
	virtual bool listen();


	};

