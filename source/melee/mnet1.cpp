#include <allegro.h>
#include "../melee.h"
REGISTER_FILE
#include "mnet1.h"
#include "mview.h"
#include "mcontrol.h"
#include "mcbodies.h"
#include "../scp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
	if (size > max_message_length + sizeof(GameEvent)) tw_error("GameEventMessage - message overflow");
	//if (source == Game::channel_server) tmp += sprintf(tmp, "Server says: ");
	//else if (source == Game::channel_client) tmp += sprintf(tmp, "Client says: ");
	if (source == Game::channel_server) c = 13; else c = 9;
	int s = size - sizeof(GameEvent);
	memcpy(tmp, message, s);
	tmp[s] = 0;
	::message.out(buffy, 6000, c);
}

void GameEventChangeLag::execute( int source ) {STACKTRACE
	if (source != Game::channel_server) return;
	if (old_lag != game->lag_frames) return;
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
int write_length_code (int max, int *clen, int len, unsigned char *where) {STACKTRACE
	if (len <= 0) tw_error( "write_length_code -- bad length");
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

void NetLog::init() {STACKTRACE
	remote_time = 0;
	ping = -1;
	log_transmitted = NULL;
	need_to_transmit = false;
	net.init();
	Log::init();
	}
void NetLog::deinit() {STACKTRACE
	net.deinit();
	}
NetLog::~NetLog() {STACKTRACE
	free (log_transmitted);
	}

void NetLog::send_packet() {STACKTRACE
	int i, j, k, pos = 8;
	if (!net.isConnected()) tw_error("NetLog::send_packet() - no connection!");
	for (i = 0; i < log_num; i += 1) if (log_dir[i] & direction_write) {
		j = log_len[i] - log_transmitted[i];
		if (j > 0) {
			buffy[pos] = i & 255;
			if (i > 255) tw_error ("NetLog::send_packet - channel # exceeds 8 bits");
			pos += 1;
			write_length_code(12, &k, j, &buffy[pos]);
			pos += k;
			memcpy(&buffy[pos], log_data[i] + log_transmitted[i], j);
			log_transmitted[i] += j;
			pos += j;
			}
		}
	if (pos <= 8) {
		tw_error ("NetLog::send_packet -- no data to send");
		return;
		}
	buffy[0] = (pos >> 0) & 255;
	buffy[1] = (pos >> 8) & 255;
	buffy[2] = (pos >> 16) & 255;
	buffy[3] = (pos >> 24) & 255;
	int actual_time = get_time();
	buffy[4] = (actual_time >> 0) & 255;
	buffy[5] = (actual_time >> 8) & 255;
	buffy[6] = (remote_time >> 0) & 255;
	buffy[7] = (remote_time >> 8) & 255;
	net.send(pos, &buffy);
	need_to_transmit = false;
	return;
	}
void NetLog::recv_packet() {STACKTRACE
	int pos, len;
	int i, j, k, l;
	len = net.recv(4, 4, &buffy);
	if (len != 4) tw_error ("NetLog::recv_packet -- net.recv error (1)");
	len = buffy[0] + (buffy[1] << 8) + (buffy[2] << 16) + (buffy[3] << 24);
	if (len & 0x80000000) {
		handle_code(len);
		return;
		}
	net.recv(4, 4, &buffy[4]);
	remote_time = buffy[4] + (buffy[5] << 8);
	int actual_time = get_time();
	ping = (actual_time - (buffy[6] + (buffy[7] << 8))) & 65535 ;
	len -= 8;
	if (len != net.recv(len, len, &buffy)) tw_error( "NetLog::recv_packet -- net.recv error (2)");
	pos = 0;
	while (pos < len) {
		i = buffy[pos];
		if (i < 0) tw_error("NegLog::recv_packet - data came in on a negative channel %d", i);
		if (i >= log_num) expand_logs(i+1);
		if (!(log_dir[i] & direction_read)) tw_error("NetLog::recv_packet -- data on wrong channel %d", i);
		pos += 1;
		l = read_length_code(len-pos, &j, &k, &buffy[pos]);
		if (l < 0) tw_error ("NetLog::recv_packet -- read_length_code failed");
		pos += j;
		if (i >= log_num) expand_logs(i+1);
		if (!(log_dir[i] & direction_read)) tw_error( "recieved data on wrong channel");
		Log::_log(i, &buffy[pos], k);
		pos += k;
		}
	if (pos != len) tw_error ("NetLog::recv_packet -- missaligned packet!!!");
	return;
	}

void NetLog::send_code(unsigned int code) {STACKTRACE
	if (!(code & 0x80000000)) tw_error ("NetLog::send_special -- bade code!");
	net.send(sizeof(unsigned int), &code);
	return;
	}
void NetLog::send_message(char *string) {STACKTRACE
	int i;
	i = sprintf((char*)&buffy[4], "remote: %s", string);
	if (i > 1000) tw_error ("net1_send_message -- message length exceeds maximum");
	buffy[0] = i & 255;
	buffy[1] = (i >> 8) & 255;
	buffy[2] = (NET1_CODE_MESSAGE >> 16) & 255;
	buffy[3] = (NET1_CODE_MESSAGE >> 24) & 255;
	net.send(i+4, &buffy);
	message.out((char*)&buffy[4], 5000, 9);
	return;
	}
void NetLog::handle_code(unsigned int code) {STACKTRACE
	switch (code & 0xF0000000) {
		case NET1_CODE_QUIT & 0xF0000000: {
			switch (code) {
				case NET1_CODE_QUIT: {
					message.out("Quitting");
					tw_error ("Quit: recieved quit signal over network");
					}
				break;
				}
			}
		break;
		case NET1_CODE_MESSAGE: {
			int len = code - NET1_CODE_MESSAGE;
			if (len > 1000) tw_error ("net1_handle_code -- message length exceeds maximum");
			net.recv(len, len, &buffy);
			buffy[len] = 0;
			message.out((char*)buffy, 5000, 10);
			}
		break;
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
void NetLog::_log(int channel, const void *data, int size) {STACKTRACE
	if (channel >= log_num) expand_logs(channel+1);
	Log::_log(channel, data, size);
	need_to_transmit = true;
	if (log_dir[channel] &  direction_immediate) send_packet();
	return;
	}
void NetLog::_unlog(int channel, void *data, int size) {
	while (ready(channel) < size) {
		if (game) game->idle();
		else idle(1);
	}
	Log::_unlog(channel, data, size);
	return;
	}
void NetLog::log_file(const char *fname) {STACKTRACE
	if (!(log_dir[channel_file_data] & direction_write)) {
			while (file_ready(fname, NULL) == -1) {
			if (game) game->idle(1);
			else idle(1);
		}
	}
	Log::log_file(fname);
	return;
	}

void NetLog::flush() {STACKTRACE
	if (!need_to_transmit) return;
	while (!net.ready2send()) {
		if (game) game->idle();
		else idle(1);
	}
	send_packet();
	}

bool NetLog::listen() {STACKTRACE
	bool b = false;
	while (net.ready2recv()) {
		recv_packet();
		b = true;
		}
	return b;
	}

int NetLog::ready(int channel) {STACKTRACE
	if (net.ready2recv()) recv_packet();
	return Log::ready(channel);
	}
