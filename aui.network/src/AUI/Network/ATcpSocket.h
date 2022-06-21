/*
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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
