//#define NETWORK_NONE
//uncoment previous line to disable networking

#include "get_time.h"
// not nice, that this module depends on timewarp this way, but well... I want to use "idle"


#if defined NETWORK_NONE
	//networking disabled
#elif defined _WIN32
#	define   NETWORK_WINSOCK
#	include "winsock.h"
#elif defined __BEOS__                  //networking disabled on BeOS (might work)
#	define NETWORK_NONE
//#	define   NETWORK_BSD
//#	define   NETWORK_NO_DNS
//#	define   NETWORK_NO_INCLUDE
//#	include "socket.h"
//#	define close_socket(a) closesocket(a)
#elif defined DJGPP                     //networking disabled on DOS at the moment (might work with libsocket)
#	define   NETWORK_NONE
//#	define NETWORK_BSD
//#	include <dos.h>
//#	include <lsck/lsck.h>
#elif defined(linux) || defined(LINUX)  //Slackware seems to work normally, but other Linuxes don't quite match BSD sockets (or the Linux man pages, for that matter)
#	define   NETWORK_BSD
#	define   NETWORK_WEIRD_SIGNEDNESS
#elif defined(UNIX) || defined(unix)
#	define   NETWORK_BSD
#else
#	define   NETWORK_NONE
#endif


#include <string.h>
#include <stdio.h>
#include "net_tcp.h"
#if defined(NETWORK_BSD) && !defined(NETWORK_NO_INCLUDE)
#	include <stdlib.h>
#	include <sys/ioctl.h>
#	include <unistd.h>
#	include <sys/time.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netdb.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <arpa/nameser.h>
#	include <fcntl.h>
#	include <errno.h>
#endif


#if defined NETWORK_BSD
#	define SOCKET int
#	define INVALID_SOCKET -1
#	define get_error() (errno)
#	define set_error(a) (errno = a)
#	if !defined(close_socket)
#		define close_socket(a) close(a)
#	endif
#	define SOCKET_ERROR -1
#	define s signed_socket
#elif defined NETWORK_WINSOCK
#	define get_error() WSAGetLastError()
#	define set_error(a) WSASetLastError(a)
#	define close_socket(a) closesocket(a)
#	define s unsigned_socket
#endif




const char *NetTCP::network_type() {
#	if defined NETWORK_WINSOCK
		return "Winsock 1 TCP";
#	elif defined NETWORK_BSD
		return "BSD sockets TCP";
#	elif defined NETWORK_NONE
		return "TCP support disabled";
#	else
		return "Unknown TCP interface";
#	endif	
	}


#ifndef NETWORK_NONE

#ifdef NETWORK_WINSOCK
static int network_status = 0;
#endif

#define NETWORK_STATUS_WINSOCK_STARTED 0x01

//msg and message exists for debug purposes.  currently it does nothing.  
static void msg (char *) {}
void (* NetTCP::message)(char*) = &msg;



void set_nonblocking(SOCKET &sd)
{
	u_long no_block = 1;
#	ifdef NETWORK_WINSOCK
	ioctlsocket(sd, FIONBIO, &no_block);
#	else
	ioctl(sd, FIONBIO, &no_block);
#	endif
}


static fd_set read_set, write_set, except_set;
static timeval tmv = {0, 0};
int NetTCP::init() {
#	if defined NETWORK_WINSOCK
		if (! (network_status & NETWORK_STATUS_WINSOCK_STARTED)) {
			WSADATA info;
			if (WSAStartup (MAKEWORD(1,1), &info) != 0) {
				message("NetTCP::init: winsock startup failed");
				return -1;
				}
			message ("NetTCP::init: winsock started");
			network_status |= NETWORK_STATUS_WINSOCK_STARTED;
			}
		else message ("NetTCP::init: winsock already started");
#	endif
	s = INVALID_SOCKET;
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&except_set);


	NBuffer = 0;
	MaxBuffer = 4096;
	buffer = (char*) malloc(MaxBuffer);

	return 0;
	}

void NetTCP::deinit() {
	message ("NetTCP::deinit: network shut down");
	if (isConnected()) disconnect();
	
	free(buffer);
	buffer = 0;

	return;
	}



void NetTCP::optimize4bandwidth() {
#	ifdef NETWORK_WINSOCK
		BOOL tmp = 0;
		setsockopt ( s, IPPROTO_TCP, TCP_NODELAY, (const char *)&tmp, sizeof(tmp));
#	endif
	//I think this might work on unix systems too, need to check
	return;
	}

void NetTCP::optimize4latency() {
#	ifdef NETWORK_WINSOCK
		int tmp = 1;
		setsockopt ( s, IPPROTO_TCP, TCP_NODELAY, (const char *)&tmp, sizeof(tmp));
#	endif
	//I think this might work on unix systems too, need to check
	return;
	}

int NetTCP::listen(int port, int (*cancel_callback)()) {
	if (s != INVALID_SOCKET) throw ("NetTCP::listen: socket already active");
	if (port <= 0) throw "NetTCP::listen - Bad port number!";
	s = socket( AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		message("NetTCP::listen: socket allocation failed");
		return -1;
		}
	message ("NetTCP::listen: socket allocated");
	sockaddr_in self;
	self.sin_family = AF_INET;
	self.sin_port = htons(port);
	self.sin_addr.s_addr = 0;
	memset(&(self.sin_zero), 0, 8);
	if (bind (s, (sockaddr *) &self, sizeof(sockaddr_in) ) == -1) {
		char tmp[80];
		sprintf (tmp, "NetTCP::listen: bind to port %d failed", port);
		message (tmp);
		close_socket(s);
		s = INVALID_SOCKET;
		return -2;
		}
	else {
		char tmp[80];
		sprintf (tmp, "NetTCP::listen: socket bound to port %d", port);
		message (tmp);
		}
	if (::listen(s, 1) == -1) {
		message ("NetTCP::listen: listen failed");
		close_socket(s);
		s = INVALID_SOCKET;
		return -3;
		}
	sockaddr_in other;
#	ifdef NETWORK_WINSOCK 
		int tmp;
#	elif defined NETWORK_BSD
#		if defined NETWORK_WEIRD_SIGNEDNESS
			//WTF!?! 
			size_t tmp; 
			//some linuxes (but not the version superdave uses (slackware of some 
			//kind)) require this, even though some linux documentation contradicts 
			//it and it differs from the BSD standard, and it's incompatible with some 
			//unices
#		else
			int tmp;
#		endif
#	endif
	SOCKET new_socket;
	while (true) {
		while (!ready2recv()) if (cancel_callback && cancel_callback()) {
			close_socket(s);
			s = INVALID_SOCKET;
			return -4;
			}
		tmp = sizeof(sockaddr_in);
		if ((new_socket = accept(s, (sockaddr *) &other, &tmp)) != INVALID_SOCKET) {
			close_socket(s);
			s = new_socket;

			set_nonblocking(s);

			// added GEO.
			char *str;
			str = inet_ntoa(other.sin_addr);	// rewrites the address into string-dotted-format
			strcpy(addr, str);

			return 0;
			}
		}
	throw("NetTCP::listen: WTF?");

	return 0;
	}


int NetTCP::connect(const char *address, int port, int (*cancel_callback)()) {
	if (port > 65535) throw "NetTCP::connect - bad port number";
	if (s != INVALID_SOCKET) throw ("NetTCP::connect: already active");
	if (!address) throw "NetTCP::connect -- NULL address!";

	sockaddr_in other;
	memset(&other,0,sizeof(other));
	other.sin_family = AF_INET;
	other.sin_port = htons((unsigned short) port);

#	ifdef NETWORK_NO_DNS
		if ((other.sin_addr.s_addr = inet_addr(address)) == -1) {
			message("NetTCP::connect: inet_addr failed");
			return -2;
			}
#	else
		hostent *he = gethostbyname(address);
		if (!he) {
			message("NetTCP::connect: target lookup failed");
			return -2;
			}
		if (he->h_addrtype != AF_INET) {
			message("NetTCP::connect: target lookup failed strangely");
			return -2;
			}
		memcpy((char *)&other.sin_addr, he->h_addr, he->h_length);
#	endif
	message("NetTCP::connect: target lookup succeeded");

	s = socket( AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		message("NetTCP::connect: socket allocation failed");
		return -3;
		}
	message ("NetTCP::connect: socket allocated");

	while (::connect(s, (struct sockaddr *)&other, sizeof(other)) == SOCKET_ERROR) {
		if (cancel_callback && cancel_callback()) {
			close_socket(s);
			s = INVALID_SOCKET;
			return -4;
			}
		}

	set_nonblocking(s);

	return 0;
	}

int NetTCP::disconnect() {
	close_socket(s);
	s = INVALID_SOCKET;
	return 0;
	}


int NetTCP::ready2send() {
	int r;
	tmv.tv_sec = 0;
	tmv.tv_usec = 0;
	FD_SET ( s, &write_set);
	r = select (s+1, &read_set, &write_set, &except_set, &tmv);
	if (r == SOCKET_ERROR) throw "NetTCP::select - error";
	FD_CLR ( s, &write_set);
	return r;
	}


int NetTCP::ready2recv() {
	int r;
	tmv.tv_sec = 0;
	tmv.tv_usec = 0;
	FD_SET ( s, &read_set);
	r = select (s+1, &read_set, &write_set, &except_set, &tmv);
	FD_CLR ( s, &read_set);
	if (r == SOCKET_ERROR) throw "NetTCP::select - error";
	return r;
	}


// add L bytes to the buffer
int NetTCP::add2buffer(char *data, int N)
{
	while (NBuffer + N > MaxBuffer)
	{
		MaxBuffer *= 2;
		buffer = (char*) realloc(buffer, MaxBuffer);
	}

	memcpy(&buffer[NBuffer], data, N);

	NBuffer += N;

	return NBuffer;
}


// remove L bytes from the buffer (from the start of the buffer, so, move the rest of the buffer
int NetTCP::rembuffer(int N)
{
	if (N > NBuffer)
		throw("Trying to remove more buffered bytes then present");

	memmove(buffer, &buffer[N], NBuffer-N);
	NBuffer -= N;

	return NBuffer;
}


int NetTCP::buflen()
{
	return NBuffer;
}


int NetTCP::sendattempt()
{
	int tmp;

	if (NBuffer <= 0)
		return 0;

	if (ready2send())
		tmp = ::send(s, buffer, NBuffer, 0);
	else
		tmp = 0;

	if (tmp == SOCKET_ERROR) throw "NetTCP::send - error";

	rembuffer(tmp);

	return tmp;
}


int NetTCP::sendall()
{
	while ( buflen() > 0 )
	{
		sendattempt();

		idle(1);
	}

	if (buflen() < 0) throw "NetTCP::send - trying to send more data than requested";

	return buflen();
}


int NetTCP::recv(int min, int max, void *data) {
	int sofar = 0;
	while (1) {
		int tmp = ::recv(s, ((char*) data) + sofar, max - sofar, 0);
		if (tmp > 0) sofar += tmp;
		if (sofar >= min) return sofar;
		if (tmp == -1) { // debuging, remove  //update: maybe don't remove?
			int i = get_error();
			static char buffy[256];
			if ((i == 10054) || (i == 10053)) {
				s = INVALID_SOCKET;
				sprintf(buffy, "The other side disconnected");
				}
			else sprintf(buffy, "NetTCP::recv - error %d", i);
			throw buffy;
			}
		//change
/*		else if (tmp == 0) {
			s = INVALID_SOCKET;
			throw "NetTCP::recv: Remote Disconnect";
			}*/
		if (tmp == SOCKET_ERROR) throw "NetTCP::recv - socket error";
		if (tmp == 0) throw "NetTCP::recv other side disconnected";
		idle(1);
		}
	}
bool NetTCP::isConnected() {
	if (s == INVALID_SOCKET) return false;
	return true;
	}









/*

// stuf useful for server ? It uses an extra socket, the listener, to check for
// incoming connection requests.


int NetTCP2::SetupListener(int port, int (*cancel_callback)())
{

	int i;
	for (i = 0; i < MaxSockets; ++i )
	{
		server_buffer[i].inp.N = 0;
		server_buffer[i].out.N = 0;
		sd[i] = INVALID_SOCKET;
	}

	if (sd_listen != INVALID_SOCKET) throw ("NetTCP2::listen: socket already active");
	if (port <= 0) throw "NetTCP2::listen - Bad port number!";
	sd_listen = socket( AF_INET, SOCK_STREAM, 0);
	if (sd_listen == INVALID_SOCKET) {
		message("NetTCP2::listen: socket allocation failed");
		return -1;
		}
	message ("NetTCP2::listen: socket allocated");

	sockaddr_in self;
	self.sin_family = AF_INET;
	self.sin_port = htons(port);
	self.sin_addr.s_addr = 0;
	memset(&(self.sin_zero), 0, 8);

	if (bind (sd_listen, (sockaddr *) &self, sizeof(sockaddr_in) ) == -1) {
		char tmp[80];
		sprintf (tmp, "NetTCP2::listen: bind to port %d failed", port);
		message (tmp);
		close_socket(sd_listen);
		sd_listen = INVALID_SOCKET;
		return -2;
		}
	else {
		char tmp[80];
		sprintf (tmp, "NetTCP2::listen: socket bound to port", port);
		message (tmp);
		}
	if (::listen(sd_listen, 1) == -1) {
		message ("NetTCP2::listen: listen failed");
		close_socket(sd_listen);
		sd_listen = INVALID_SOCKET;
		return -3;
		}

//	set_as_host();		// show that this computer runs the host...

	return 1;
}



int NetTCP2::CloseListener()
{
	close_socket(sd_listen);	// closing the listening socket.
	return 1;
}


*/




#else

int NetTCP::init() {
	return 0;
	}
void NetTCP::deinit() {
	}
int NetTCP::listen(int port, int (*cancel_callback)()) {
	throw("Networking is disabled in this program.\nProbably you should try a windows or unix version instead.");
	return -1;
	}
int NetTCP::connect(const char *address, int port, int (*cancel_callback)()) {
	throw("Networking is disabled in this program.\nProbably you should try a windows or unix version instead.");
	return -1;
	}
int NetTCP::disconnect() {
	return 0;
	}
int NetTCP::ready2send() {
	return 0;
	}
int NetTCP::ready2recv() {
	return 0;
	}
//int NetTCP::send(int size, const void *data) {
//	return 0;
//	}
int NetTCP::recv(int min, int max, void *data) {
	return 0;
	}
void NetTCP::optimize4bandwidth() {
	return;
	}
void NetTCP::optimize4latency() {
	return;
	}
bool NetTCP::isConnected() {
	return true;
	}


#endif

#ifdef get_error
#undef get_error
#endif
#ifdef set_error
#undef set_error
#endif
#ifdef close_socket
#undef close_socket
#endif
#ifdef s
#undef s
#endif
