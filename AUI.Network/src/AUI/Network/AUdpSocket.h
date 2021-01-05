/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
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

#include <cstdint>
#include <AUI/Common/AString.h>
#include <AUI/Network.h>


#include "AAbstractSocket.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/IO/IOException.h"
#include "AUI/IO/IOutputStream.h"

#include "AInet4Address.h"

class AByteBuffer;

class API_AUI_NETWORK AUdpSocket : public AAbstractSocket
{
public:

    /**
     * \brief Creates socket
     * \param port port
     */
    AUdpSocket(uint16_t port);


    /**
     * \brief Creates socket with random port (used for "client" socket)
     */
	AUdpSocket();

	AUdpSocket(const AUdpSocket&) = delete;
    ~AUdpSocket() override = default;


    /**
     * \brief Sends data by address.
     * \param buf data buffer
     * \param dst destination delivery address
     */
	void write(const AByteBuffer& buf, const AInet4Address& dst);

	/**
	 * \brief Read data.
	 * \param buf (out) received data
	 * \param dst (out) sender address
	 */
	void read(AByteBuffer& buf, AInet4Address& dst);

protected:
	int createSocket() override;
	
private:
	AInet4Address mSelf;

};
