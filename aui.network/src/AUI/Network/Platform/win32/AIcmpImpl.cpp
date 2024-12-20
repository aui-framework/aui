/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
                    AUI_ASSERT(handle != INVALID_HANDLE_VALUE);
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

            AUI_ASSERT(r == 0);
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
