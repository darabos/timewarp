/* $Id$ */ 

#ifndef __NET_TCP_H__
#define __NET_TCP_H__

class NetTCP {
	public:

		char addr[512];

		//init() is called once on each NetTCP before it is used
		int init();
		void deinit();
		//listen listens on a port for a connection.  used by servers.  if cancel_callback returns true then it will abort
		int listen(int port, int (*cancel_callback)() = NULL);
		//connect connects to a remote computer which is listening.  used by clients.  if cancel_callback returns true then it will abort
		int connect(const char *address, int port, int (*cancel_callback)() = NULL);
		//disconnect hangs up a connection
		int disconnect();
		//ready2send returns true if send() can return quickly
		int ready2send();
		//ready2recv returns true if recv() can return quickly
		int ready2recv();
		//send (size, data) sends size bytes of data
		int sendall();
		//recv (min, max, data) recieves at least min bytes, and at most max bytes, storing them in data
		int recv(int min, int max, void *data);
		//optimizes for bandwidth (good for file transfers)
		void optimize4bandwidth();
		//optimizes for latency (good for games)
		void optimize4latency();
		//returns true if connected
		bool isConnected();
		//describes the network type
		static const char *network_type();

		int sendattempt();
		// the buffer used for subsequent send(attempts)
		// adds N bytes to the buffer
		// it's probably advantageous if each socket can manage its own buffer
		int add2buffer(char *data, int N);
		
		// remove N bytes from the buffer
		int rembuffer(int N);

		// returns the buffer length
		int buflen();

	private:
		union {
			int signed_socket;
			unsigned int unsigned_socket;
			};
		static void (*message)(char *txt);


		char *buffer;
		int MaxBuffer, NBuffer;
	};


#endif // __NET_TCP_H__
