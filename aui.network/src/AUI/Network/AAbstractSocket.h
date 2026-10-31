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

#include <chrono>

#include <AUI/Network.h>

#include "AInet4Address.h"
#include "AUI/Common/AString.h"

/**
 * ---
 * title: AAbstractSocket
 * icon: material/connection
 * ---
 *
 * @brief Base class for all sockets.
 * @ingroup network
 * @details
 * Holds the native socket handle and the local address. The handle is released when the socket is destroyed, or earlier
 * with `close()`. Sockets are not copyable.
 *
 * Failures are reported with exceptions derived from [AIOException]. A blocked thread that is interrupted (see
 * `AThread::interrupt`) while waiting in a socket call gets `AThread::Interrupted`, which is the way to stop a thread
 * blocked in [ATcpSocket] or [AUdpSocket] reads.
 *
 * Concrete sockets: [ATcpSocket], [ATcpServerSocket], [AUdpSocket].
 */
class API_AUI_NETWORK AAbstractSocket {
private:
    int mHandle = 0;
    AInet4Address mSelfAddress;

protected:
    static AString getErrorString();

    [[nodiscard]] inline int getHandle() const { return mHandle; }

    static void handleError(const AString& message, int code);

    AAbstractSocket(int handle, const AInet4Address& selfAddress) : mHandle(handle), mSelfAddress(selfAddress) {}

    /**
     * @brief Initialise socket
     */
    void init();

    /**
     * @brief Binds socket for port. Used for ATcpServerSocket and AUdpSocket
     * @param bindingPort port
     */
    void bind(uint16_t bindingPort);

    /**
     * @brief Binds socket to the given address and port. Port 0 lets the system pick a free port; the picked port is
     * available through getAddress() afterwards.
     */
    void bind(const AInet4Address& bindingAddress);

    /**
     * @brief Creates socket handle.
     */
    virtual int createSocket() = 0;

public:
    AAbstractSocket();
    AAbstractSocket(const AAbstractSocket&) = delete;

    virtual ~AAbstractSocket();

    void close();

    /**
     * @brief Sets the receive timeout.
     * @details
     * A blocked read that waits longer than `timeout` fails. Zero disables the timeout.
     */
    void setTimeout(std::chrono::milliseconds timeout);

    /**
     * @brief Waits until a read would not block.
     * @param timeout how long to wait; zero just checks
     * @return true if there is data to read, or the peer has closed the connection (read returns 0)
     * @throws AIOException if the socket is closed or broken
     */
    [[nodiscard]] bool waitForRead(std::chrono::milliseconds timeout);

    const AInet4Address& getAddress() const { return mSelfAddress; }
};
