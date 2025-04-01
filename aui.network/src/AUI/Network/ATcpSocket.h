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

#include <AUI/Network.h>

#include "AAbstractSocket.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/IO/IOutputStream.h"

#include "AInet4Address.h"

class AByteBuffer;

/**
 * @brief A bidirectional TCP connection (either a client connection or returned by ATcpServerSocket).
 * @ingroup network
 */
class API_AUI_NETWORK ATcpSocket : public AAbstractSocket, public IInputStream, public IOutputStream {
    friend class ATcpServerSocket;

public:
    ATcpSocket(const AInet4Address& destinationAddress);

    ~ATcpSocket() override;

    size_t read(char* dst, size_t size) override;
    void write(const char* buffer, size_t size) override;

protected:
    ATcpSocket(int handle, const AInet4Address& selfAddr) : AAbstractSocket(handle, selfAddr) {}

    int createSocket() override;
};
