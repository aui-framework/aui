﻿/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
