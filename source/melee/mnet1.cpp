#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "../melee.h"
REGISTER_FILE
#include "mnet1.h"
#include "mview.h"
#include "mcontrol.h"
//#include "mcbodies.h"
#include "../scp.h"


static int log_show_num = 0;

GameEventMessage::GameEventMessage (const char *text)
{STACKTRACE
	int l = strlen(text);
	if (l > max_message_length) l = max_message_length;
	memcpy(message, text, l);
	size = sizeof(GameEvent) + l;
	type = Game::event_message;
}
void GameEventMessage::execute( int source ) {STACKTRACE
	char buffy[64+max_message_length];
	char *tmp = buffy;
	int c = 15;
	if ((unsigned short int)size > max_message_length + sizeof(GameEvent)) { tw_error("GameEventMessage - message overflow"); }
	//if (source == Game::channel_server) tmp += sprintf(tmp, "Server says: ");
	//else if (source == Game::channel_client) tmp += sprintf(tmp, "Client says: ");
	if (source == channel_server) c = 13; else c = 9;
	int s = size - sizeof(GameEvent);
	memcpy(tmp, message, s);
	tmp[s] = 0;
	::message.out(buffy, 6000, c);
}

void GameEventChangeLag::execute( int source )
{
	STACKTRACE;

	if (source != channel_server)
		return;
	if (old_lag != game->lag_frames)
		return;

	if (new_lag > old_lag)
		game->increase_latency(new_lag - old_lag);
	if (new_lag < old_lag)
		game->decrease_latency(old_lag - new_lag);
	/*
	int i;
	for (i = old_lag; i != new_lag; ) {
		if (i < new_lag) {
			game->increase_latency();
			i += 1;
		}
		else if (i > new_lag) {
			game->decrease_latency();
			i -= 1;
		}
	}
	*/
}



int read_length_code (int max, int *clen, int *len, unsigned char *where) {STACKTRACE
	if (max < 1) return -1;
	*clen = 1;
	*len = where[0];
	if (*len < 255) return 0;
	*clen = 3;
	if (*clen > max) return -1;
	*len = where[1] + (where[2] << 8);
	if (*len < 65535) return 0;
	*clen = 7;
	if (*clen > max) return -1;
	*len = where[3] + (where[4] << 8) + (where[5] << 16) + (where[6] << 24);
	return 0;
	}
int write_length_code (int max, int *clen, int len, unsigned char *where) {
	STACKTRACE;
	if (len <= 0) { tw_error( "write_length_code -- bad length"); }
	if (max < 1) return -1;
	if (len < 255) {
		where[0] = len;
		*clen = 1;
		return 0;
		}
	else {
		if (max < 3) return -1;
		where[0] = 255; 
		if (len < 65535) {
			where[1] = len & 255;
			where[2] = len >> 8;
			*clen = 3;
			return 0;
			}
		else {
			if (max < 7) return -1;
			where[1] = 255;
			where[2] = 255;
			where[3] = (len >> 0) & 255;
			where[4] = (len >> 8) & 255;
			where[5] = (len >> 16) & 255;
			where[6] = (len >> 24) & 255;
			*clen = 7;
			return 0;
			}
		}
	}


void NetLog::init()
{
	STACKTRACE;

	log_transmitted = NULL;
	need_to_transmit = false;

	int i;
	for ( i = 0; i < max_connections; ++i )
	{
		net[i].init();

		remote_time[i] = 0;
		ping[i] = -1;
	}

	num_connections = 0;	// number of active connections.

	buffy_size = 16 * 4096;
	//buffy = new unsigned char [buffy_size];
	buffy = 0;
	buffy = (unsigned char*) realloc(buffy, buffy_size);

	Log::init();
}


void NetLog::deinit()
{
	STACKTRACE;

	int i;
	for ( i = 0; i < max_connections; ++i )
		net[i].deinit();

	//delete buffy;
	free(buffy);

}

NetLog::~NetLog()
{
	STACKTRACE;

	free (log_transmitted);
}


/** \brief fills buffy with contents that can be sent out to connections. These contents
are a sequence of new stuff that's waiting in the log-buffer.
*/

void NetLog::check_bufsize(int size)
{
	int oldsize = buffy_size;

	while (size > buffy_size)
		buffy_size *= 2;

	if (buffy_size > oldsize)
	{
		tw_error("Warning: buffer size is too small [%i] needed [%i]", oldsize, buffy_size);
		buffy = (unsigned char *) realloc( buffy, buffy_size);
	}

	// ERROR - weird, this can't be done ??
	
}


void NetLog::prepare_packet()
{
	int i, j, k;

	int pos = 8;

	int totsize = 0;
	for (i = 0; i < log_num; i += 1) if (log_dir[i] & direction_write)
	{
		j = log_len[i] - log_transmitted[i];
		totsize += j;

		if (j > 0)
		{

			check_bufsize(pos+1);
			if (i > 255) { tw_error ("NetLog::send_packet - channel # exceeds 8 bits"); }
			buffy[pos] = i & 255;
			pos += 1;

			check_bufsize(pos+8);
			write_length_code(12, &k, j, &buffy[pos]);
			pos += k;

			check_bufsize(pos+j);
			memcpy(&buffy[pos], log_data[i] + log_transmitted[i], j);
			log_transmitted[i] += j;
			pos += j;
		}
	}
//	if (pos <= 8) {
//		tw_error ("NetLog::send_packet -- no data to send");
//		return;
//	}
	buffy[0] = (pos >> 0) & 255;
	buffy[1] = (pos >> 8) & 255;
	buffy[2] = (pos >> 16) & 255;
	buffy[3] = (pos >> 24) & 255;
	int actual_time = get_time();
	buffy[4] = (actual_time >> 0) & 255;
	buffy[5] = (actual_time >> 8) & 255;

	//buffy_num = pos;

	// Add the packet to the buffers of each connection.
	// but only if there's content, otherwise you're sending just overhead and no actual data...
	if (totsize > 0)
	{

		int conn;
		for ( conn = 0; conn < num_connections; ++conn )
		{
			// maintain some connection-dependent control parameter (namely,
			// the remote time ? This is the time-value that's sent here, and
			// is bounced back without being changed.
			buffy[6] = (remote_time[conn] >> 0) & 255;
			buffy[7] = (remote_time[conn] >> 8) & 255;

			//message.print(1500, 14, "add2buffer[%i]", pos);
			//message.animate(0);
			net[conn].add2buffer((char*)buffy, pos);
		}
	}
}

/** \brief This waits till the whole buffer is sent into the net
*/
void NetLog::send_packet_block(int conn)
{
	STACKTRACE;

	if (!net[conn].isConnected()) { tw_error("NetLog::send_packet() - no connection!"); }
	net[conn].sendall();	// note, the data are buffered in prepare_packet()
}

/** \brief This tries to send the buffer onto the net, but doesn't wait if it doesn't succeed.
If it doesn't succeed, it'll keep data in a memory buffer until the next attempt.
*/
void NetLog::send_packet_noblock(int conn)
{
	STACKTRACE;

	if (!net[conn].isConnected()) { tw_error("NetLog::send_packet() - no connection!"); }
	net[conn].sendattempt();	// note, the data are buffered in prepare_packet()
}


/** \brief Sends copies of a writeable packet of information to all open connections. This
does not guarantee they're actually sent at that very moment ?
I think you also need to call this from within idle(), so that it'll proceed even if
the game is halted for a while ...
*/

void NetLog::send_packets()
{
	STACKTRACE;

	prepare_packet();

	int conn;

	for ( conn = 0; conn < num_connections; ++conn )
	{
		send_packet_block(conn);
	}

	need_to_transmit = false;

	return;
}


// receive packets from a particular connection

void NetLog::recv_packet(int conn)
{
	STACKTRACE;
	int pos, len;
	int i, j, k, l;
	
	
	//len = net[conn].recv(4, 4, &buffy);
	len = net[conn].recv(4, 4, buffy);
	
	if (len != 4) { tw_error ("NetLog::recv_packet -- net.recv error (1)"); }
	len = buffy[0] + (buffy[1] << 8) + (buffy[2] << 16) + (buffy[3] << 24);
	
	if (len & 0x80000000) {
		//handle_code(len);
		tw_error("NetLog::recv_packet - this shouldn't happen");
		return;
	}
	
	net[conn].recv(4, 4, &buffy[4]);
	
	remote_time[conn] = buffy[4] + (buffy[5] << 8);
	int actual_time = get_time();
	ping[conn] = (actual_time - (buffy[6] + (buffy[7] << 8))) & 65535 ;
	len -= 8;

	if (len == 0)
		tw_error("Unexpected: receiving packet with overhead, but zero content!");

	//if (len != net[conn].recv(len, len, &buffy))
	if (len != net[conn].recv(len, len, buffy))
		tw_error( "NetLog::recv_packet -- net.recv error (2)");
	
	// data've been received, now put them into the log structure.
	
	pos = 0;
	while (pos < len) {
		i = buffy[pos];
		if (i < 0) { tw_error("NegLog::recv_packet - data came in on a negative channel %d", i); }
		if (i >= log_num) expand_logs(i+1);
		if (!(log_dir[i] & direction_read)) { tw_error("NetLog::recv_packet -- data on wrong channel %d", i); }
		pos += 1;
		
		check_bufsize(pos+8);
		l = read_length_code(len-pos, &j, &k, &buffy[pos]);
		if (l < 0) { tw_error ("NetLog::recv_packet -- read_length_code failed"); }
		pos += j;
		if (i >= log_num) expand_logs(i+1);
		if (!(log_dir[i] & direction_read)) { tw_error( "recieved data on wrong channel"); }

		check_bufsize(pos+k);
		Log::_log(i, &buffy[pos], k);
		pos += k;
	}
	if (pos != len)
	{
		tw_error ("NetLog::recv_packet -- missaligned packet!!!");
	}
	
	return;
	
}



void NetLog::expand_logs(int num_channels) {STACKTRACE
	int old_log_num = log_num;
	Log::expand_logs(num_channels);
	if (log_num) {
		log_transmitted = (int*)  realloc(log_transmitted, sizeof(int) * log_num);
		for (int i = old_log_num; i < log_num; i += 1) {
			log_transmitted[i] = 0;
			}
		}
	else {
		log_transmitted = NULL;
		}
	return;
	}


void NetLog::_log(int channel, const void *data, int size)
{
	STACKTRACE;

	if (log_show_data)
	{
		message.print(1500, 15, "[%i]_log: ch[%i] size[%i]", log_show_num, channel, size);
		message.animate(0);
		++log_show_num;
	}

	if (channel >= log_num) expand_logs(channel+1);
	Log::_log(channel, data, size);
	need_to_transmit = true;
	if (log_dir[channel] &  direction_immediate) send_packets();
	//send_packets();	// if you use a non-blocking socket, you don't have to wait, do you ?
	return;
}


/** \brief Wait till the requested data have arrived
*/
void NetLog::_unlog(int channel, void *data, int size)
{
	if (log_show_data)
	{
		message.print(1500, 15, "[%i]_unlog: ch[%i] size[%i]", log_show_num, channel, size);
		message.animate(0);
		++log_show_num;
	}

	while (ready(channel) < size) {
		//if (game) game->idle();
		//else idle(1);
		idle_extra(1);
	}
	Log::_unlog(channel, data, size);
	return;
	}

void NetLog::log_file(const char *fname)
{
	STACKTRACE;

	/*
	if (!(log_dir[channel_file_data] & direction_write)) {
		message.out("log_file -- sharing filename"); message.animate(0);
			while (file_ready(fname, NULL) == -1) {
			//if (game) game->idle(1);
			//else idle(1);
			idle_extra(1);
		}
	}
	message.out("log_file -- log filename"); message.animate(0);
	*/
	Log::log_file(fname);
	//message.out("log_file -- done"); message.animate(0);
	return;
	}

/** \brief sends packets to all outgoing connections; waits till they're ready for
sending, therefore it is guaranteed that the data are actually sent onto the net. This
blocks program flow.
*/

void NetLog::flush_block()
{
	STACKTRACE;

	if (log_show_data)
	{
		message.print(1500, 13, "FLUSH");
		message.animate(0);
	}

	if (!need_to_transmit)	// if immediate transmission isn't required, flush_block changes into a flush_noblock
		flush_noblock();

	prepare_packet();

	int conn;
	for ( conn = 0; conn < num_connections; ++conn )		
	{
		if (net[conn].buflen() <= 0)
			continue;

		/* this check / loop is already in send_packet... so no need to do it twice
		while (!net[conn].ready2send())
		{
			//if (game) game->idle();
			//else idle(1);
			idle(1);
		}
		*/
		
		send_packet_block(conn);
	}
}


/** \brief Sends data if possible, ignoring data that can't be sent yet. Those data remain
in the buffer until the next call to this subroutine. This does not block program flow.
*/
void NetLog::flush_noblock()
{
	STACKTRACE;

	if (log_show_data)
	{
		message.print(1500, 13, "FLUSH");
		message.animate(0);
	}

	prepare_packet();

	int conn;
	for ( conn = 0; conn < num_connections; ++conn )		
		send_packet_noblock(conn);
}




bool NetLog::listen()
{
	STACKTRACE;
	bool b = false;

	if (log_show_data)
	{
		message.print(1500, 13, "LISTEN");
		message.animate(0);
	}

	int conn;
	// receive packets from all connections
	
	for ( conn = 0; conn < num_connections; ++conn )
	{
		while (net[conn].ready2recv())
		{
			recv_packet(conn);
			b = true;

			idle_extra(1);
		}
	}

	return b;
	}


void NetLog::recv_noblock()
{
	int conn;
	// receive packets from all connections, if possible.
	
	for ( conn = 0; conn < num_connections; ++conn )
	{
		if (net[conn].ready2recv()) recv_packet(conn);
	}
}

int NetLog::ready(int channel)
{
	STACKTRACE;


	recv_noblock();

	return Log::ready(channel);
	
}



bool NetLog::ready2send()
{
	int conn;
	for ( conn = 0; conn < num_connections; ++conn )
	{
		if (!net[conn].ready2send())
			return false;
	}
	return true;
}




bool NetLog::add_listen(int port)
{
	// repeat till the next player connects

	while (!net[num_connections].isConnected())
	{
		int i;
		i = net[num_connections].listen(port, is_escape_pressed);

		if (i)
			return false;

		idle(1);
	}

	++num_connections;

	return true;
}



bool NetLog::add_connect(const char *address, int port)
{
	while (!net[num_connections].isConnected())
	{
		int i;
		i = net[num_connections].connect(address, port, is_escape_pressed);
		
		if (i)
			return false;

		idle(1);
	}

	++num_connections;
	return true;
}


char *NetLog::get_address(int n)
{
	return net[n].addr;
}



void NetLog::optimize4latency()
{
	int conn;
	// receive packets from all connections
	
	for ( conn = 0; conn < num_connections; ++conn )
	{
		net[conn].optimize4latency();
	}
}




static int netlog_idletime = 0;

void NetLog::use_idle(int time)
{
	/* IN PRINCIPLE YOU NEED THIS ...
	const int interval = 50;

	netlog_idletime += time;
	if (netlog_idletime < interval)
		return;

	netlog_idletime -= interval;

	int conn;
	bool readyall = true;
	for ( conn = 0; conn < num_connections; ++conn )
	{
		if (net[conn].ready2recv() != 0)
			recv_packet(conn);

		if (net[conn].ready2send() == 0)
			readyall = false;
	}

	if (readyall)
		send_packets();
		*/
}








