// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <cassert>
#include <AUI/Network/ATcpSocket.h>

#include "Exceptions.h"

#if AUI_PLATFORM_WIN
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

size_t ATcpSocket::read(char* dst, size_t size)
{
	int res = recv(getHandle(), dst, size, 0);
	if (res < 0) {
		handleError("socket read error", res);
	}
	return res;
}

void ATcpSocket::write(const char* buffer, size_t size)
{
	int res = send(getHandle(), buffer, size, 0);
	if (res < 0) {
		handleError("socket write error", res);
	}
	assert(res == size);
}

int ATcpSocket::createSocket()
{
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}
