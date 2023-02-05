// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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


#include "AAbstractSocket.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/IO/AIOException.h"
#include "AUI/IO/IOutputStream.h"

#include "AInet4Address.h"
#include "AUI/Common/AByteBufferView.h"

class AByteBuffer;

/**
 * @brief A bidirectional UDP socket.
 * @ingroup network
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
