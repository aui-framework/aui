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
