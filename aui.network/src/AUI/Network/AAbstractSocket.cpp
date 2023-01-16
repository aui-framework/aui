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

#include "AAbstractSocket.h"


#include "Exceptions.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/AEOFException.h"
#include "AUI/IO/AIOException.h"
#include "AUI/Thread/AThread.h"


#if AUI_PLATFORM_WIN
#include <ws2tcpip.h>
#include <windows.h>
#include <AUI/Logging/ALogger.h>

void aui_wsa_init()
{
	class WSA
	{
	private:
		WSADATA wsa;

	public:
		WSA()
		{
			if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
				throw AIOException((AString("Failed. Error code: ") + AString::number(WSAGetLastError())).c_str());
			}
		}
		~WSA()
		{
			WSACleanup();
		}
	};
	static WSA wsa;
}
#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <netdb.h>
#include <AUI/Logging/ALogger.h>

#endif

void AAbstractSocket::init()
{
#if AUI_PLATFORM_WIN
	aui_wsa_init();
	if ((mHandle = createSocket()) == INVALID_SOCKET) {
		throw AIOException(
			(AString("Failed to create ASocket. Error code: ") + AString::number(WSAGetLastError())).c_str());
	}
#else
	if ((mHandle = createSocket()) < 0)
	{
		throw AIOException("Failed to create ASocket.");
	}
#endif

	static const int buflen = 1 << 24;
	if (setsockopt(mHandle, SOL_SOCKET, SO_RCVBUF, (char*)& buflen, sizeof(buflen)) < 0 ||
		setsockopt(mHandle, SOL_SOCKET, SO_SNDBUF, (char*)& buflen, sizeof(buflen)) < 0)
		throw AIOException(
		(AString("Failed to setsockopt:") + getErrorString()).c_str());
}

AString AAbstractSocket::getErrorString()
{
#if AUI_PLATFORM_WIN
	int error = WSAGetLastError();
	wchar_t* str;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, static_cast<DWORD>(error), 0,
	               (LPWSTR)&str, 0, nullptr);
	AString s;
	switch (error)
	{
	case WSAETIMEDOUT:
		s = "ETIMEDOUT (connection timeout)";
		break;
	case WSAECONNRESET:
		s = "ECONNRESET (connection reset)";
		break;
	default:
		s = AString::number(error);
	}
	return s + ": " + AString(str);
#else
	return strerror(errno);
#endif
}

void AAbstractSocket::handleError(const AString& message, int code)
{
	AString msg = message + ": " + getErrorString();
#if AUI_PLATFORM_WIN
	switch (WSAGetLastError()) {
	case WSAEINTR:
		throw AThread::Interrupted();
	case WSAECONNRESET:
		throw AEOFException();
	default:
		throw SocketException(msg);
	}
#else
    switch (code) {
        case EINTR:
            throw AThread::Interrupted();
        case ECONNRESET:
            throw AEOFException();
        default:
            throw SocketException(msg);
    }
#endif
}

void AAbstractSocket::bind(uint16_t bindingPort)
{
	mSelfAddress = AInet4Address(0u, bindingPort);
	auto addr = mSelfAddress.addr();
	for (unsigned i = 5; i >= 0; --i) {
        const int res = ::bind(getHandle(), reinterpret_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
        if (res < 0) {
            if (i == 0) {
                ALogger::err("failed to bind to port: " + AString::number(bindingPort) + ", giving up.");
                handleError("failed to bind to port: " + AString::number(bindingPort), res);
            } else {
                ALogger::err("failed to bind to port: " + AString::number(bindingPort) + ", trying again");
                AThread::sleep(std::chrono::seconds(3));
            }
        } else {
            break;
        }
    }
}

AAbstractSocket::AAbstractSocket()
{
}


AAbstractSocket::~AAbstractSocket()
{
	if (mHandle)
		close();
}

void AAbstractSocket::close()
{
	if (mHandle) {
#if AUI_PLATFORM_WIN
		closesocket(mHandle);
#else
		shutdown(mHandle, 2);
#endif
		mHandle = 0;
	}
}


void AAbstractSocket::setTimeout(int secs) {
	struct timeval tv;

	tv.tv_sec = secs * 1000;
	tv.tv_usec = 0;
	if (setsockopt(getHandle(), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) < 0) {
		throw AIOException(AString("setsockopt error ") + getErrorString());
	}
}