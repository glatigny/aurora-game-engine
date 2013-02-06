#include "network_engine.h"


PacketNetworkEngine::PacketNetworkEngine(unsigned int newMtu) : mtu(newMtu)
{
	xerror(mtu > 0, "Cannot use a zero MTU.");
	xadvice(mtu <= 65536, "Performance : MTU should not be more than 64k to avoid too large intermediate buffers.");
}

PacketNetworkEngine::~PacketNetworkEngine() {}

// QC:A (TODO : connected() not implemented)
bool PacketNetworkEngine::send(NetSocket* socket, const char* message)
{
	xerror(socket, "NULL socket.");
	unsigned int length = getPackedStructureLength(message);
//	xerror(connected(socket), "Socket not connected.");

	// Split the message in blocks of the size of the MTU.
	while(length > mtu) {
		if(!send(socket, message, mtu))
			return false;
		message += mtu;
		length -= mtu;
	}

	// Send the last part.
	return send(socket, message, length);
}

// QC:A (TODO : connected() not implemented)
const char* PacketNetworkEngine::receive(NetSocket* socket)
{
	xerror(socket, "NULL socket.");
	xassert(dynamic_cast<PacketNetSocket*>(socket), "Socket is not a PacketNetSocket.");
//	xerror(connected(socket), "Socket not connected.");

	PacketNetSocket* psocket = (PacketNetSocket*)socket;

	if(psocket->messageReady()) {
		psocket->popFirstMessage();
	}

	char *recvbuf = (char*)alloca(mtu);
	size_t received;
	do {
		received = receive(socket, recvbuf, mtu);
		psocket->appendData(recvbuf, received);
	} while(received);
	
	if(psocket->messageReady()) {
		return psocket->messageBuffer;
	}

	return NULL;
}

