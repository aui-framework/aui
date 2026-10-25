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

#include "AAbstractSocket.h"


#include "Exceptions.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/AEOFException.h"
#include "AUI/IO/AIOException.h"
#include "AUI/Thread/AThread.h"
#include "AUI/Platform/ErrorToException.h"


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
#include <poll.h>
#include <netinet/in.h>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
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
	return aui::impl::formatSystemError(error).description;
#else
	return strerror(errno);
#endif
}

void AAbstractSocket::handleError(const AString& message, [[maybe_unused]] int code)
{
#if !AUI_PLATFORM_WIN
    // code is the return value of the failed syscall (-1); the actual reason is errno. getErrorString() may clobber it.
    const int error = errno;
#endif
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
    switch (error) {
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
	bind(AInet4Address(0u, bindingPort));
}

void AAbstractSocket::bind(const AInet4Address& bindingAddress)
{
	mSelfAddress = bindingAddress;
	auto addr = mSelfAddress.addr();
	const auto portString = bindingAddress.toString();
	for (unsigned i = 5; i >= 0; --i) {
        const int res = ::bind(getHandle(), reinterpret_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
        if (res < 0) {
            if (i == 0) {
                ALogger::err("failed to bind to: " + portString + ", giving up.");
                handleError("failed to bind to: " + portString, res);
            } else {
                ALogger::err("failed to bind to: " + portString + ", trying again");
                AThread::sleep(std::chrono::seconds(3));
            }
        } else {
            break;
        }
    }

	// the actual port is assigned by the system when binding to port 0.
	sockaddr_in actual {};
	socklen_t actualLength = sizeof(actual);
	if (getsockname(getHandle(), reinterpret_cast<sockaddr*>(&actual), &actualLength) == 0) {
		mSelfAddress = AInet4Address(actual);
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
		::close(mHandle);
#endif
		mHandle = 0;
	}
}


bool AAbstractSocket::waitForRead(std::chrono::milliseconds timeout) {
    if (mHandle == 0) {
        throw AIOException("socket is closed");
    }
#if AUI_PLATFORM_WIN
    WSAPOLLFD fd {};
    fd.fd = static_cast<SOCKET>(mHandle);
    fd.events = POLLRDNORM;
    const int res = WSAPoll(&fd, 1, static_cast<INT>(timeout.count()));
#else
    pollfd fd {};
    fd.fd = mHandle;
    fd.events = POLLIN;
    const int res = poll(&fd, 1, static_cast<int>(timeout.count()));
#endif
    if (res < 0) {
        throw AIOException(AString("poll error ") + getErrorString());
    }
    if (res == 0) {
        return false;
    }
    if (fd.revents & POLLNVAL) {
        throw AIOException("socket is closed");
    }
    return true;   // readable, or POLLHUP / POLLERR, which a read reports
}

void AAbstractSocket::setTimeout(std::chrono::milliseconds timeout) {
#if AUI_PLATFORM_WIN
	// winsock expects milliseconds as DWORD
	const DWORD tv = static_cast<DWORD>(timeout.count());
#else
	struct timeval tv;
	tv.tv_sec = static_cast<decltype(tv.tv_sec)>(timeout.count() / 1000);
	tv.tv_usec = static_cast<decltype(tv.tv_usec)>((timeout.count() % 1000) * 1000);
#endif
	if (setsockopt(getHandle(), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) < 0) {
		throw AIOException(AString("setsockopt error ") + getErrorString());
	}
}