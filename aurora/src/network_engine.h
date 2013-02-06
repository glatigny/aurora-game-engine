#ifndef NETWORK_ENGINE_H
#define NETWORK_ENGINE_H

#if 0
// Very high level interface. Will be implemented later.
class NetworkSession {
	virtual ~NetworkSession() {}
};

class NetworkHub {
	virtual ~NetworkHub() {}
};

class NetworkEngineInterface {
public:
	/** Create a new session.
	 * @param sessioName the name of the session.
	 * @param listen set to true to open a listening port. When in listening mode, the server mode will be enabled, which relays
	 * packets to other people in the same hub and to other connected servers.
	 * @return an object representing the created session.
	 * @see terminateSession to free the object.
	 */
	virtual NetworkSession* createSession(const char* sessionName, bool listen = false) = 0;

	/** Terminate a session.
	 * @param session the session to terminate. The object will be deleted by this function.
	 */
	virtual void terminateSession(NetworkSession* session) = 0;

	/** Add a server to a session
	 * @param session the network session to add the server to.
	 * @param address the real address of the server. IP addresses, hostnames or any equivalent address is accepted.
	 * @param username the username too connect as.
	 * @param token the authentication token.
	 */
	virtual void addServer(NetworkSession* session, const char* address, const char* username, const char* token) = 0;

	/** Join a Hub
	 * @param session the Network Session.
	 * @param hubId the identifier of the hub.
	 * @return the network hub object.
	 */
	virtual NetworkHub* join(NetworkSession* session, const char* hubId) = 0;

	/** Part from a hub
	 * @param hub the hub to part from.
	 */
	virtual void part(NetworkHub* hub) = 0;

	/** Send a message
	 * @param hub the hub to send data to.
	 * @parem channel the channel that carries packets.
	 * @param data the data to send.
	 * @param length the length of the data structure.
	 */
	virtual void send(NetworkHub* hub, unsigned int channel, void* data, size_t length) = 0;

	/** Receive a message
	 * @param hub the hub to receive data from.
	 * @param channel the channel that carries packets.
	 * @param data outputs a pointer to a buffer allocated by the function. The caller has to AOEFREE() the buffer to avoid memory leaks.
	 * @return the amount of read data, or 0 if no data is available.
	 */
	virtual size_t receive(NetworkHub* hub, unsigned int channel, void* &data) = 0;

	virtual unsigned int reserveChannel(NetworkHub* hub) = 0;

	virtual void freeChannel(NetworkHub* hub, unsigned int channel) = 0;
};

#endif

struct NetSocket; // Common base for socket structure. Platform-dependant members will be added by inheritance.

class NetworkEngineInterface {
public:
	virtual ~NetworkEngineInterface() {}

	/** Connect a socket to an URI.
	 * This function tries to connect a socket to a specific URI.
	 * The URI may represent a single endpoint or a multicast system, depending on its nature and.
	 * Protocol is named after the game.
	 *
	 * Example :
	 *  NetSocket* socket = netEngine.connect("yeppoh://game.yeppoh.com");
	 *  char* packet = packStructure("s", "Hello World");
	 *	netEngine.send(socket, packet);
	 *	packet = netEngine.receive(socket);
	 *
	 * @param uri the URI to connect the socket to.
	 * @return the connected socket, or NULL in case of failure.
	 */
	virtual NetSocket* connect(const char* uri) = 0;

	/** Disconnect a socket.
	 * When disconnected, the pointer to the socket is freed and thus made invalid.
	 * YOU CANNOT REUSE THE SOCKET AFTER IT HAS BEEN DISCONNECTED.
	 * @param socket the socket to disconnect.
	 */
	virtual void disconnect(NetSocket* socket) = 0;

	/** Send a message through a socket.
	 * The message MUST be a sized structure. Use packStructure to create correctly formatted buffers.
	 * Unlike packets, messages are correctly delimited. Messages are guaranteed to be received in the same order and to be of the same size as when they were sent.
	 * @param socket the socket used to send data.
	 * @param message the packed structure to send through the socket. It must be a sized structure.
	 * @return true if the message was sent successfully, false otherwise. It does not guarantee that the other peers successfully received the message even when returning true.
	 */
	virtual bool send(NetSocket* socket, const char* message) = 0;

	/** Receive a message from a socket.
	 * Unlike packets, messages are correctly delimited. Messages are guaranteed to be received in the same order and to be of the same size as when they were sent.
	 *
	 * Call to this function is non-blocking. It just returns the next message in the receive queue or NULL if this queue is empty.
	 *
	 * @param socket the socket to read from.
	 * @return the pointer to the next packet in the queue or NULL if the queue is empty. The pointer is guaranteed to be valid until the next call to receive or until the NetworkEngine object is deleted.
	 */
	virtual const char* receive(NetSocket* socket) = 0;

	/** Tells whether the specified socket is connected.
	 * @return true if the socket is connected, false otherwise.
	 */
	virtual bool connected(NetSocket* socket) = 0;
};

struct PacketNetSocket;

/** Common implementation for packed-based networks like TCP.
 * Just derivate from this class and implement protected methods.
 * The class will split and manage messages into packets that fit in the MTU. It only works for protocols that guarantee correct order and no packet loss.
 * It may be used over UDP or raw ethernet with a reliability layer plugged inbetween.
 */
class PacketNetworkEngine : public NetworkEngineInterface {
private:
	PacketNetworkEngine();
	unsigned int mtu;

protected:

	/** Send a packet.
	 * The packet size will be inferior or equal to the MTU.
	 * @param socket the socket to send the packet to.
	 * @param packet payload buffer.
	 * @param packetSize the size of the packet to be sent.
	 * @return true if the message was sent successfully, false otherwise. It does not guarantee that the other peers successfully received the message even when returning true.
	 */
	virtual bool send(NetSocket* socket, const char* packet, size_t packetSize) = 0;

	/** Receive a packet.
	 * This function can be blocking.
	 * @param socket the socket to receive data from.
	 * @param packet a pointer to the buffer to store the incoming packet.
	 * @param bufSize the size of the reveive buffer.
	 * @return the number of received bytes or 0 if the queue is empty or in case of error.
	 */
	virtual size_t receive(NetSocket* socket, char* packet, size_t bufSize) = 0;

public:
	/** Constructor.
	 * @param mtu the MTU of the underlying network. It is used to setup receive buffers and the high leve code guarantees that send() will never be called with a size greater than the MTU.
	 */
	PacketNetworkEngine(unsigned int mtu);

	virtual ~PacketNetworkEngine();

	/* NetworkEngineInterface */

	virtual bool send(NetSocket* socket, const char* message);
	virtual const char* receive(NetSocket* socket);
};

struct NetSocket {
	virtual ~NetSocket() {}
};

/** A packet socket structure.
 */
struct PacketNetSocket : public NetSocket {
	char* messageBuffer;
	char* messageBufferPointer;
	unsigned int messageBufferSize;

	PacketNetSocket(unsigned int bufferSize) :
		messageBuffer((char*)AOEMALLOC(bufferSize)),
		messageBufferPointer(messageBuffer),
		messageBufferSize(bufferSize)
	{
		xerror(messageBuffer, "Cannot allocate memory for receive buffer.");
	}

	virtual ~PacketNetSocket() {
		AOEFREE(messageBuffer);
	}

	unsigned int nextMessageLength() {
		xassert(messageBufferPointer >= messageBuffer, "messageBufferPointer points before the buffer.");
		xassert(messageBufferPointer < messageBuffer + messageBufferSize, "messageBufferPointer points after the buffer.");
		if(messageBuffer && (messageBufferPointer - messageBuffer) >= 4) {
			return getPackedStructureLength(messageBuffer);
		} else {
			return 0;
		}
	}

	bool messageReady() {
		xassert(messageBufferPointer >= messageBuffer, "Misplaced messageBufferPointer.");
		if(messageBufferPointer == messageBuffer)
			return false;
		return (unsigned int)(messageBufferPointer - messageBuffer) >= nextMessageLength();
	}

	void appendData(const char* data, unsigned int length) {
		if(!length)
			return;

		if(messageBufferPointer + length > messageBuffer + messageBufferSize) {
			resize((messageBufferPointer - messageBuffer) + length + 64);
		}

		memcpy(messageBufferPointer, data, length);
		messageBufferPointer += length;
	}

	/** Remove the first message in front of the queue. Remaining data is not lost. */
	void popFirstMessage() {
		xerror(messageReady(), "Popping incomplete message.");
		
		unsigned int msgLen = nextMessageLength();
		xassert(msgLen > 0, "Zero-length message.");
		memmove(messageBuffer, messageBuffer + msgLen, (messageBufferPointer - messageBuffer) - msgLen);
		messageBufferPointer -= msgLen;
	}

	void resize(unsigned int newSize) {
		unsigned int ptrDelta = messageBufferPointer - messageBuffer;
		messageBuffer = (char*)realloc(messageBuffer, newSize);
		messageBufferPointer = messageBuffer + ptrDelta;
	}
};

#endif /* NETWORK_ENGINE_H */
