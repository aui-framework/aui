// AUI Framework - Declarative UI toolkit for modern C++17
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

#include "ATcpServerSocket.h"

#if AUI_PLATFORM_WIN
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

	return aui::ptr::manage(new ATcpSocket(s, addr));
}

ATcpServerSocket::ATcpServerSocket(uint16_t serverPort)
{
	init();
	bind(serverPort);
}
