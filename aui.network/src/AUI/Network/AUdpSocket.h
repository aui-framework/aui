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

#include <cstdint>
#include <AUI/Common/AString.h>
#include <AUI/Network.h>


#include "AAbstractSocket.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/IO/AIOException.h"
#include "AUI/IO/IOutputStream.h"

#include "AInet4Address.h"
#include "AUI/Common/AByteBufferView.h"

class AByteBuffer;

/**
 * ---
 * title: AUdpSocket
 * icon: material/email-fast
 * ---
 *
 * @brief A bidirectional UDP socket.
 * @ingroup network
 * @details
 * UDP is connectionless: every `write` sends one datagram to an explicit destination and every `read` receives one
 * datagram along with the sender's address. Delivery and ordering are not guaranteed.
 *
 * ```cpp
 * AUdpSocket socket(9000);          // receiver, bound to port 9000
 * AByteBuffer buffer;
 * AInet4Address sender;
 * socket.read(buffer, sender);      // blocks until a datagram arrives
 * socket.write(buffer, sender);     // reply
 * ```
 *
 * The datagram size is limited to 32767 bytes. See [network-udp].
 */
class API_AUI_NETWORK AUdpSocket : public AAbstractSocket
{
public:

    /**
     * @brief Creates socket
     * @param port port
     */
    AUdpSocket(uint16_t port);


    /**
     * @brief Creates socket with random port (used for "client" socket)
     */
	AUdpSocket();

	AUdpSocket(const AUdpSocket&) = delete;
    ~AUdpSocket() override = default;


    /**
     * @brief Sends data by address.
     * @param buf data buffer
     * @param dst destination delivery address
     */
	void write(AByteBufferView buf, const AInet4Address& dst);

	/**
	 * @brief Read data.
	 * @param buf (out) received data
	 * @param dst (out) sender address
	 */
	void read(AByteBuffer& buf, AInet4Address& dst);

protected:
	int createSocket() override;
	
private:
	AInet4Address mSelf;

};
