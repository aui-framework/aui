/*
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

#include <AUI/Network.h>

#include "AInet4Address.h"
#include "AUI/Common/AString.h"

/**
 * @brief Base class for all sockets.
 * @ingroup network
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
     * @brief Creates socket handle.
     */
    virtual int createSocket() = 0;

public:
    AAbstractSocket();
    AAbstractSocket(const AAbstractSocket&) = delete;

    virtual ~AAbstractSocket();

    void close();
    void setTimeout(int secs);

    const AInet4Address& getAddress() const { return mSelfAddress; }
};
