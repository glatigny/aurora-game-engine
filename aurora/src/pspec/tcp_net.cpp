#include "tcp_net.h"

#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef WIN32
typedef int ssize_t;
#endif

// fcntl(sockfd, F_SETFL, O_NONBLOCK);

TCPNetworkEngine::TCPNetworkEngine() : PacketNetworkEngine(4096)
{
#ifdef WIN32
	WORD wVersionRequested, wStartUp;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2,0);
	if( (wStartUp = WSAStartup(wVersionRequested, &wsaData) ) != 0)
		return;
#endif
}

#ifdef WIN32
TCPNetworkEngine::~TCPNetworkEngine()
{
	WSACleanup();
}
#endif

NetSocket* TCPNetworkEngine::connect(const char* uri)
{
	if(strncmp(uri, "yc://", 5) == 0)
	{
		const char* host = uri + 5;

		TCPNetSocket* s = new TCPNetSocket();
		s->socket = socket(PF_INET, SOCK_STREAM, 0);

		xerror(s->socket != SOCKET_ERROR, "Could not create a socket.");

		struct hostent* hostaddr = gethostbyname(host);
		xwarn(hostaddr->h_addr_list, "Could not retreive address for host %s", host);
		if(!hostaddr->h_addr_list)
			return NULL;

		struct sockaddr_in saddr;
		bzero(&saddr, sizeof(saddr));
		saddr.sin_family = hostaddr->h_addrtype;
		saddr.sin_port = htons(port);
#ifndef WIN32
		inet_pton(hostaddr->h_addrtype, hostaddr->h_addr_list[0], &saddr.sin_addr);
#else
		struct sockaddr_storage ss;
		int sslen = sizeof(ss);
		WSAStringToAddressA(hostaddr->h_addr_list[0], hostaddr->h_addrtype, NULL, (struct sockaddr*)&ss, &sslen);
		saddr.sin_addr = ((struct sockaddr_in *)&ss)->sin_addr;
#endif

		if(::connect(s->socket, (struct sockaddr *)&saddr, sizeof(saddr) ) == SOCKET_ERROR)
		{
			xwarn(false, "Could not connect to %s", host);
			return NULL;
		}

#ifdef WIN32
		ULONG on=1;
		ioctlsocket(s->socket, FIONBIO, &on);
#else
		fcntl(s->socket, F_SETFL, O_NONBLOCK);
#endif

		return s;
	} else {
		xwarn(false, "Unknown protocol : %s", uri);
		return NULL;
	}
}

void TCPNetworkEngine::disconnect(NetSocket* socket)
{
#ifdef WIN32
	closesocket(((TCPNetSocket*)socket)->socket);
#else
	close(((TCPNetSocket*)socket)->socket);
#endif
}

bool TCPNetworkEngine::connected(NetSocket* socket)
{
	return false; // TODO
}

bool TCPNetworkEngine::send(NetSocket* socket, const char* packet, size_t packetSize)
{
	return ::send(((TCPNetSocket*)socket)->socket, packet, packetSize, 0) == (ssize_t)packetSize;
}

size_t TCPNetworkEngine::receive(NetSocket* socket, char* packet, size_t bufSize)
{
	ssize_t result = recv(((TCPNetSocket*)socket)->socket, packet, bufSize, 0);
	xassert(result <= (ssize_t)bufSize, "Received too much data");
	if(result > 0)
		return result;
	return 0;
}

