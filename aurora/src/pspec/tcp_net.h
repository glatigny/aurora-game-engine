#ifndef TCP_NET_H
#define TCP_NET_H

#include "network_engine.h"

#ifdef WIN32
extern "C" {
//#  include <winsock2.h>
//#  include <windows.h>
}
#  define socklen_t int
#  define inet_netof(a) inet_addr(inet_ntoa(a))
#else
#  include <netdb.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <sys/uio.h>
#  include <sys/ioctl.h>
#  include <unistd.h>
#  define SOCKET int
#  define SOCKET_ERROR (-1)
#endif

#define SOCKETS_TCP_MODE	(0)
#define SOCKETS_UDP_MODE	(1)

#define SOCKETS_OK			(0)
#define SOCKETS_ERR			(1)
#define SOCKETS_BAD_SOCK	(2)
#define SOCKETS_BAD_IP		(3)
#define SOCKETS_ERR_CONNECT	(4)
#define SOCKETS_ERR_LISTEN	(5)
#define SOCKETS_ERR_CLOSE	(6)
#define SOCKETS_ERR_BROADC	(7)
#define SOCKETS_ERR_SEND	(8)
#define SOCKETS_TRUE		(9)
#define SOCKETS_FALSE		(10)


struct TCPNetSocket : public PacketNetSocket {
public:
	TCPNetSocket() : PacketNetSocket(1536) {}
	SOCKET socket;
	virtual ~TCPNetSocket() {}
};

class TCPNetworkEngine : public PacketNetworkEngine {
private:
	static const unsigned int port = 16719;
protected:
	/* PacketNetworkEngine */

	virtual bool send(NetSocket* socket, const char* packet, size_t packetSize);
	virtual size_t receive(NetSocket* socket, char* packet, size_t bufSize);

public:
	TCPNetworkEngine();
	virtual ~TCPNetworkEngine()
#ifdef WIN32
	;
#else
	{}
#endif

	/* NetworkEngineInterface */

	virtual NetSocket* connect(const char* uri);
	virtual void disconnect(NetSocket* socket);
	virtual bool connected(NetSocket* socket);
};

#endif /* TCP_NET_H */
