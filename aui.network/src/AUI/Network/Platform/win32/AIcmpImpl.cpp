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

//
// Created by Alex2772 on 1/27/2023.
//

#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <winternl.h>
#include <icmpapi.h>

#include <AUI/Network/AIcmp.h>
#include "AUI/Platform/win32/AWin32Event.h"
#include "AUI/Platform/win32/AWin32EventWait.h"
#include "AUI/Platform/ErrorToException.h"
#include "AUI/Platform/win32/WinIoThread.h"
#include "AUI/IO/AIOException.h"

extern void aui_wsa_init();

AFuture<std::chrono::high_resolution_clock::duration> AIcmp::ping(AInet4Address destination, std::chrono::milliseconds timeout) noexcept {
    AFuture<std::chrono::high_resolution_clock::duration> result;
    WinIoThread::enqueue([result, destination, timeout]() mutable {

        try {
            char sendData[] = "Hello";

            struct Icmp {
                HANDLE handle = IcmpCreateFile();
                AFuture<std::chrono::high_resolution_clock::duration> result;
                struct {
                    ICMP_ECHO_REPLY icmp;
                    char message[sizeof(sendData)];
                } reply;

                Icmp(AFuture<std::chrono::high_resolution_clock::duration> result) : result(std::move(result)) {
                    assert(handle != INVALID_HANDLE_VALUE);
                }

                ~Icmp() {
                    IcmpCloseHandle(handle);
                }
            };

            aui_wsa_init();

            auto icmp = new Icmp(std::move(result));
            auto r = IcmpSendEcho2(icmp->handle,
                                   nullptr, // event
                                   [](void* userdata, PIO_STATUS_BLOCK result, ULONG Reserved) {
                                       auto icmp = reinterpret_cast<Icmp*>(userdata);
                                       try {
                                           if (result->Status != 0) {
                                               throw AIOException(aui::impl::formatSystemError(result->Status).description);
                                           }
                                           icmp->result.supplyValue(
                                                   std::chrono::milliseconds(icmp->reply.icmp.RoundTripTime));
                                       } catch (...) {
                                           icmp->result.supplyException();
                                       }
                                       delete icmp;
                                   },
                                   icmp, // apc context
                                   destination.toLongAddressOnly(), // address
                                   sendData, // send data
                                   sizeof(sendData), // sizeof send data
                                   nullptr, // RequestOptions
                                   &icmp->reply, // pointer to reply buffer
                                   sizeof(icmp->reply), // sizeof reply buffer
                                   timeout.count() // timeout in ms
            );

            assert(r == 0);
            if (auto lastError = GetLastError(); lastError != ERROR_IO_PENDING) {
                delete icmp;
                throw AIOException(aui::impl::formatSystemError(lastError).description);
            }
        } catch (...) {
            result.supplyException();
        }
    });


    return result;
}
