
#ifndef _NET_TCP_H
#define _NET_TCP_H

class NetTCP {
	public:
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
		int send(int size, const void *data);
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
	private:
		union {
			int signed_socket;
			unsigned int unsigned_socket;
			};
		static void (*message)(char *txt);
	};


#endif
