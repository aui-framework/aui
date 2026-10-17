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

#pragma once

#include <array>
#include <cstdint>
#include <AUI/Common/AString.h>
#include <AUI/Network.h>

struct sockaddr_in;

/**
 * ---
 * title: AInet4Address
 * icon: material/ip-network
 * ---
 *
 * @brief Represents an ipv4 address with port.
 * @ingroup network
 * @details
 * AInet4Address is a small copyable value which is accepted by every socket class of aui::network. Use
 * `AInet4Address::fromString` to build one from a dotted address or a hostname (resolved with the system resolver,
 * throws [AIOException] if the name is unresolved) and `AInet4Address::fromArray` to build one from four octets.
 *
 * ```cpp
 * auto server = AInet4Address::fromString("example.com", 80);
 * ALogger::info("Net") << "Connecting to " << server.toString(); // prints ip:port
 * ```
 *
 * The port is passed as the second argument and is stored along with the address. Addresses can be compared with
 * `==`, `<` and `>`, so they can be used as keys of `AMap`.
 *
 * !!! warning
 *
 *     Only IPv4 is supported.
 *
 * See [network-tcp] and [network-udp] for usage with sockets.
 */
class API_AUI_NETWORK AInet4Address
{
private:
	uint32_t mAddr;
	uint16_t mPort;

public:
	uint64_t toLong() const;
	uint32_t toLongAddressOnly() const;
	AInet4Address();
	AInet4Address(const sockaddr_in& other);
	AInet4Address(const AInet4Address& other);
	AInet4Address(uint32_t ip, uint16_t port = -1);
	static AInet4Address fromArray(std::array<uint8_t, 4> ip, uint16_t port = -1);
	static AInet4Address fromString(const AString& addr, uint16_t port = -1);
	
	sockaddr_in addr() const;
	bool operator>(const AInet4Address& r) const;
	bool operator<(const AInet4Address& r) const;
	bool operator==(const AInet4Address& o) const;
	
	AString toString() const;
};
