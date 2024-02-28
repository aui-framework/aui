// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#pragma once

#include <cstdint>
#include <AUI/Common/AString.h>
#include <AUI/Network.h>

struct sockaddr_in;

/**
 * @brief Represents an ipv4 address with port.
 * @ingroup network
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
	AInet4Address(uint8_t ip[4], uint16_t port = -1);
	AInet4Address(uint32_t ip, uint16_t port = -1);
	AInet4Address(const AString& addr, uint16_t port = -1);
	
	sockaddr_in addr() const;
	bool operator>(const AInet4Address& r) const;
	bool operator<(const AInet4Address& r) const;
	bool operator==(const AInet4Address& o) const;
	
	AString toString() const;
};
