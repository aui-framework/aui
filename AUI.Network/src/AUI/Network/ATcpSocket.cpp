#include <cassert>
#include <AUI/Network/ATcpSocket.h>

#include "Exceptions.h"

#if defined(_WIN32)
#include <ws2tcpip.h>
#else

#include <sys/socket.h>
#include <netinet/in.h>

#endif


ATcpSocket::ATcpSocket(const AInet4Address& destinationAddress)
{
	init();
	auto addr = destinationAddress.addr();
	int res = connect(getHandle(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

	if (res < 0) {
		handleError("connection failed", res);
	}
}

ATcpSocket::~ATcpSocket()
{
}

int ATcpSocket::read(char* dst, int size)
{
	int res = recv(getHandle(), dst, size, 0);
	if (res < 0) {
		handleError("socket read error", res);
	}
	return res;
}

int ATcpSocket::write(const char* buffer, int size)
{
	int res = send(getHandle(), buffer, size, 0);
	if (res < 0) {
		handleError("socket write error", res);
	}
	assert(res == size);
	return res;
}

int ATcpSocket::createSocket()
{
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}
