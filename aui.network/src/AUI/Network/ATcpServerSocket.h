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
 * @brief TCP server socket.
 * @ingroup network
 */
class API_AUI_NETWORK ATcpServerSocket: public AAbstractSocket
{
protected:
	int createSocket() override;
public:
	ATcpServerSocket(uint16_t serverPort);
	~ATcpServerSocket() override;

    /**
     * Blocks the thread until next connection.
     * @return new connection
     */
	_<ATcpSocket> accept();
};
