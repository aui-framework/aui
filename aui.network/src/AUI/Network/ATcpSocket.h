// AUI Framework - Declarative UI toolkit for modern C++17
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
class API_AUI_NETWORK ATcpSocket: public AAbstractSocket, public IInputStream, public IOutputStream
{
friend class ATcpServerSocket;
public:

	ATcpSocket(const AInet4Address& destinationAddress);

	~ATcpSocket() override;

	size_t read(char* dst, size_t size) override;
	void write(const char* buffer, size_t size) override;


protected:
	ATcpSocket(int handle, const AInet4Address& selfAddr)
		: AAbstractSocket(handle, selfAddr)
	{
	}
	
	int createSocket() override;
};
