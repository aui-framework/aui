/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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
