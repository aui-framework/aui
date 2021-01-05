/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Network.h>



#include "AInet4Address.h"
#include "AUI/Common/AString.h"

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
	 * \brief Initialise socket
	 */
	void init();
	
	/**
	 * \brief Bind socket for port. Used for ATcpServerSocket and AUdpSocket
	 * \param bindingPort port
	 */
	void bind(uint16_t bindingPort);


	/**
	 * \brief Create socket handle. Use ::socket()
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
