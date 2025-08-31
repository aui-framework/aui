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

//
// Created by Alex2772 on 5/18/2022.
//

#include <Windows.h>
#include <AUI/IO/AIOException.h>
#include <AUI/Platform/ErrorToException.h>

aui::impl::Error aui::impl::formatSystemError(int errorCode) {
    if(errorCode == 0)
        return {}; //No error message has been recorded

    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

    AString message(reinterpret_cast<const char16_t*>(messageBuffer), size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return { errorCode, message.trim().removeAll('\r').removeAll('\n') };
}

aui::impl::Error aui::impl::formatSystemError() {
    return formatSystemError(GetLastError());
}

void aui::impl::lastErrorToException(AString message) {
    auto[lastErrorCode, description] = formatSystemError();
    message += ": ";
    message += description;
    switch (lastErrorCode) {
        case ERROR_FILE_NOT_FOUND:
            throw AFileNotFoundException(message);
        case ERROR_PATH_NOT_FOUND:
            throw AFileNotFoundException("{} (underlying directory does not exist)"_format(message));
        case ERROR_ACCESS_DENIED:
            throw AAccessDeniedException(message);
        default:
            throw AIOException("{} (code {})"_format(message, lastErrorCode));
    }
}
