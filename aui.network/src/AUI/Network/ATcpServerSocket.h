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
#include "AAbstractSocket.h"
#include "ATcpSocket.h"


/**
 * ---
 * title: ATcpServerSocket
 * icon: material/server-network
 * ---
 *
 * @brief TCP server socket.
 * @ingroup network
 * @details
 * Binds to the given port on all interfaces upon construction and hands out an [ATcpSocket] per incoming connection.
 *
 * ```cpp
 * ATcpServerSocket server(8080);
 * for (;;) {
 *     _<ATcpSocket> client = server.accept(); // blocks
 *     // talk to the client
 * }
 * ```
 *
 * `accept()` blocks, so a server normally runs it in a dedicated thread (see [AThread]) and serves each client
 * from a worker. See [network-tcp].
 */
class API_AUI_NETWORK ATcpServerSocket: public AAbstractSocket
{
protected:
	int createSocket() override;
public:
	/**
	 * @brief Listens on all interfaces.
	 * @param serverPort port to listen; 0 lets the system pick a free port (see `getAddress()`).
	 */
	ATcpServerSocket(uint16_t serverPort);

	/**
	 * @brief Listens on the given address, i.e., `AInet4Address::fromArray({127, 0, 0, 1}, 0)` for loopback only.
	 * @details
	 * Port 0 lets the system pick a free port. The actual address is available through `getAddress()`.
	 */
	ATcpServerSocket(const AInet4Address& bindAddress);
	~ATcpServerSocket() override;

    /**
     * @brief Blocks the thread until next connection.
     * @details
     * Throws if the server socket was closed from another thread, which is the way to stop a thread blocked here.
     * @return new connection
     */
	_<ATcpSocket> accept();
};
