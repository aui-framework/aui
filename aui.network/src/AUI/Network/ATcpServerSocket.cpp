/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
	sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int s = ::accept(getHandle(), reinterpret_cast<sockaddr*>(&addr), &addrlen);
	if (s < 0) {
		handleError("socket accept error", s);
	}

	return aui::ptr::manage_shared(new ATcpSocket(s, addr));
}

ATcpServerSocket::ATcpServerSocket(uint16_t serverPort): ATcpServerSocket(AInet4Address(0u, serverPort))
{
}

ATcpServerSocket::ATcpServerSocket(const AInet4Address& bindAddress)
{
	init();
#if !AUI_PLATFORM_WIN
	// allows restarting a server right after the previous instance has gone.
	const int reuse = 1;
	setsockopt(getHandle(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
	bind(bindAddress);
	int res = listen(getHandle(), SOMAXCONN);
	if (res < 0) {
		handleError("socket listen error", res);
	}
}
