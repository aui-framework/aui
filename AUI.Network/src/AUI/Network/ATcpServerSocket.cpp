#include "ATcpServerSocket.h"

#if defined(_WIN32)
#include <ws2tcpip.h>
#else

#include <sys/socket.h>
#include <netinet/in.h>

#endif


int ATcpServerSocket::createSocket()
{
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

ATcpServerSocket::~ATcpServerSocket()
{
}

_<ATcpSocket> ATcpServerSocket::accept()
{
	int res = listen(getHandle(), SOMAXCONN);
	if (res < 0)
		handleError("socket listen error", res);

	sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int s = ::accept(getHandle(), reinterpret_cast<sockaddr*>(&addr), &addrlen);

	auto socket = new ATcpSocket(s, addr);
	return _<ATcpSocket>(socket);
}

ATcpServerSocket::ATcpServerSocket(uint16_t serverPort)
{
	init();
	bind(serverPort);
}
