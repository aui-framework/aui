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

#include "AInet4Address.h"

#include <string>
#include "AUI/IO/AIOException.h"


#if AUI_PLATFORM_WIN
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

#if AUI_PLATFORM_WIN
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

AInet4Address::AInet4Address(uint32_t ip, uint16_t port):
	mAddr(ip),
	mPort(port)
{
}

AInet4Address AInet4Address::fromArray(std::array<uint8_t, 4> ip, uint16_t port) {
    uint32_t u32ip = (static_cast<uint32_t>(ip[0]) << 24) |
                    (static_cast<uint32_t>(ip[1]) << 16) |
                    (static_cast<uint32_t>(ip[2]) << 8)  |
                    (static_cast<uint32_t>(ip[3]));
    return AInet4Address(u32ip, port);
}
AInet4Address AInet4Address::fromString(const AString& addr, uint16_t port) {
#if AUI_PLATFORM_WIN
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
        throw AIOException((AString("Unresolved hostname: ") + addr).c_str());
    auto* sockaddrin = (sockaddr_in*)result->ai_addr;

#if AUI_PLATFORM_WIN
    uint32_t u32ip = sockaddrin->sin_addr.S_un.S_addr;
#else
    uint32_t u32ip = sockaddrin->sin_addr.s_addr;
#endif

    freeaddrinfo(result);

    return AInet4Address(u32ip, port);
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
	std::snprintf(buf, sizeof(buf), "%u.%u.%u.%u:%u",
		static_cast<unsigned>(bytes[0]),
		static_cast<unsigned>(bytes[1]),
		static_cast<unsigned>(bytes[2]),
		static_cast<unsigned>(bytes[3]),
		static_cast<unsigned>(mPort)
	);
	return buf;
}

bool AInet4Address::operator==(const AInet4Address& o) const {
	return mAddr == o.mAddr && mPort == o.mPort;
}
