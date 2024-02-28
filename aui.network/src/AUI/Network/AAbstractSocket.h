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

#include <AUI/Network.h>



#include "AInet4Address.h"
#include "AUI/Common/AString.h"

/**
 * @brief Base class for all sockets.
 * @ingroup network
 */
class API_AUI_NETWORK AAbstractSocket
{	
private:
	int mHandle = 0;
	AInet4Address mSelfAddress;

	
protected:
	static AString getErrorString();

	[[nodiscard]] inline int getHandle() const
	{
		return mHandle;
	}

	static void handleError(const AString& message, int code);


	AAbstractSocket(int handle, const AInet4Address& selfAddress)
		: mHandle(handle),
		mSelfAddress(selfAddress)
	{
	}
	
	/**
	 * @brief Initialise socket
	 */
	void init();
	
	/**
	 * @brief Bind socket for port. Used for ATcpServerSocket and AUdpSocket
	 * @param bindingPort port
	 */
	void bind(uint16_t bindingPort);


	/**
	 * @brief Create socket handle. Use ::socket()
	 */
	virtual int createSocket() = 0;
	
public:
	AAbstractSocket();
	AAbstractSocket(const AAbstractSocket&) = delete;

	virtual ~AAbstractSocket();

	void close();
	void setTimeout(int secs);

	const AInet4Address& getAddress() const {
		return mSelfAddress;
	}
};
