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
#include <AUI/Network/AUdpSocket.h>
#include <AUI/Common/AByteBuffer.h>

#include "Exceptions.h"

#if AUI_PLATFORM_WIN
#include <ws2tcpip.h>
#else

#include <sys/socket.h>
#include <netinet/in.h>

#endif



AUdpSocket::AUdpSocket(uint16_t source_port) :
	mSelf(AInet4Address::fromArray({ 0, 0, 0, 0 }, source_port)) {

    auto addr = mSelf.addr();
	if (::bind(getHandle(), (sockaddr*)&addr, sizeof(sockaddr_in)) < 0) {
		throw AIOException(
			(AString("Failed to bind ASocket. Error code: ") + getErrorString()).c_str());
	}
}

AUdpSocket::AUdpSocket() :
	AUdpSocket(static_cast<uint16_t>((rand() % 20000) + 30000)) {
}


void AUdpSocket::write(AByteBufferView buf, const AInet4Address& dst) {
	assert(buf.size() < 32768);
	//static boost::mutex m;
	//boost::unique_lock lock(m);
	auto addr = dst.addr();
	if (sendto(getHandle(), buf.data(), static_cast<int>(buf.size()), 0, (sockaddr*)&addr, sizeof(sockaddr_in)) <=
        0) {
		throw AIOException(AString("sendto error ") + getErrorString());
	}
}

void AUdpSocket::read(AByteBuffer& buf, AInet4Address& dst) {
	buf.reserve(32768);
	sockaddr_in from;
	memset(&from, 0, sizeof(sockaddr_in));
	socklen_t  l = sizeof(from);
	int res;
	for (;;) {
		res = recvfrom(getHandle(), buf.data(), 32768, 0, (sockaddr*)& from, (socklen_t *)&l);
		if (res <= 0) {
			AString msg = AString("recvfrom error ") + getErrorString();
#if AUI_PLATFORM_WIN
			switch (WSAGetLastError()) {
			case WSAEINTR:
				throw AThread::Interrupted();
			case WSAECONNRESET:
				// https://stackoverflow.com/questions/30749423/is-winsock-error-10054-wsaeconnreset-normal-with-udp-to-from-localhost
				continue; // wtf why does it work?
			default:
				throw SocketException(msg, dst);
#else
            switch (res) {
                case EINTR:
                    throw AThread::Interrupted();
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
