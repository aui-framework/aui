#include "AInet4Address.h"

#include <string>
#include "AUI/IO/IOException.h"


#if defined(_WIN32)
extern void aui_wsa_init();
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <netdb.h>

#endif

uint64_t AInet4Address::toLong() const {
	uint64_t r = mAddr;
	r <<= 16;
	r |= mPort;
	return r;
}
uint32_t AInet4Address::toLongAddressOnly() const {
	return mAddr;
}

AInet4Address::AInet4Address() {
}

AInet4Address::AInet4Address(const sockaddr_in& other):

#if defined(_WIN32)
	mAddr(other.sin_addr.S_un.S_addr),
#else
        mAddr(other.sin_addr.s_addr),
#endif
	mPort(htons(other.sin_port))
{
}

AInet4Address::AInet4Address(const AInet4Address& other):
	mAddr(other.mAddr),
	mPort(other.mPort)
{
}

AInet4Address::AInet4Address(uint8_t ip[4], uint16_t port):
	mPort(port)
{
}

AInet4Address::AInet4Address(uint32_t ip, uint16_t port):
	mAddr(ip),
	mPort(port)
{
}

AInet4Address::AInet4Address(const AString& addr, uint16_t port):
	mPort(port) {
#if defined(_WIN32)
	aui_wsa_init();
#endif
	sockaddr_in sock;
	memset((char*)& sock, 0, sizeof(sock));
	std::string ports = std::to_string(port);
	addrinfo hints = { 0 };
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	addrinfo* result;

	if (getaddrinfo(addr.toStdString().c_str(), ports.c_str(), &hints, &result) != 0)
		throw IOException((AString("Unresolved hostname: ") + addr).c_str());
	auto* sockaddrin = (sockaddr_in*)result->ai_addr;

#if defined(_WIN32)
	mAddr = sockaddrin->sin_addr.S_un.S_addr;
#else
	mAddr = sockaddrin->sin_addr.s_addr;
#endif

	freeaddrinfo(result);
}


sockaddr_in AInet4Address::addr() const {
	sockaddr_in sock;
	memset((char*)& sock, 0, sizeof(sock));
	memcpy((char*)& sock.sin_addr.s_addr, &mAddr, 4);
	sock.sin_family = AF_INET;
	sock.sin_port = htons(mPort);
	return sock;
}

bool AInet4Address::operator>(const AInet4Address& r) const {
	return toLong() > r.toLong();
}

bool AInet4Address::operator<(const AInet4Address& r) const {
	return toLong() < r.toLong();
}

AString AInet4Address::toString() const {
	uint8_t* bytes = (uint8_t*)&mAddr;
	char buf[32];
	sprintf(buf, "%d.%d.%d.%d:%d", bytes[0], bytes[1], bytes[2], bytes[3], mPort);
	return buf;
}

bool AInet4Address::operator==(const AInet4Address& o) const {
	return mAddr == o.mAddr && mPort == o.mPort;
}
