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
