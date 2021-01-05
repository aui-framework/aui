/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#include <cassert>
#include <AUI/Network/AUdpSocket.h>

#include "Exceptions.h"

#if defined(_WIN32)
#include <ws2tcpip.h>
#else

#include <sys/socket.h>
#include <netinet/in.h>

#endif



AUdpSocket::AUdpSocket(uint16_t source_port) :
	mSelf({ 0, 0, 0, 0 }, source_port) {

    auto addr = mSelf.addr();
	if (::bind(getHandle(), (sockaddr*)&addr, sizeof(sockaddr_in)) < 0) {
		throw IOException(
			(AString("Failed to bind ASocket. Error code: ") + getErrorString()).c_str());
	}
}

AUdpSocket::AUdpSocket() :
	AUdpSocket(static_cast<uint16_t>((rand() % 20000) + 30000)) {
}


void AUdpSocket::write(const AByteBuffer& buf, const AInet4Address& dst) {
	assert(buf.getSize() < 32768);
	//static boost::mutex m;
	//boost::unique_lock lock(m);
	auto addr = dst.addr();
	if (sendto(getHandle(), buf.getBuffer(), static_cast<int>(buf.getSize()), 0, (sockaddr*)&addr, sizeof(sockaddr_in)) <=
		0) {
		throw IOException(AString("sendto error ") + getErrorString());
	}
}

void AUdpSocket::read(AByteBuffer& buf, AInet4Address& dst) {
	buf.reserve(32768);
	sockaddr_in from;
	memset(&from, 0, sizeof(sockaddr_in));
	socklen_t  l = sizeof(from);
	int res;
	for (;;) {
		res = recvfrom(getHandle(), buf.getBuffer(), 32768, 0, (sockaddr*)& from, (socklen_t *)&l);
		if (res <= 0) {
			AString msg = AString("recvfrom error ") + getErrorString();
#if defined(_WIN32)
			switch (WSAGetLastError()) {
			case WSAEINTR:
				throw AThread::AInterrupted();
			case WSAECONNRESET:
				// https://stackoverflow.com/questions/30749423/is-winsock-error-10054-wsaeconnreset-normal-with-udp-to-from-localhost
				continue; // wtf why does it work?
			default:
				throw SocketException(msg, dst);
#else
            switch (res) {
                case EINTR:
                    throw AThread::AInterrupted();
                default:
                    throw SocketException(msg, dst);
#endif
			}
		}
		dst = from;
		break;
	}
	buf.setSize(static_cast<size_t>(res));
}

int AUdpSocket::createSocket()
{
	return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}
