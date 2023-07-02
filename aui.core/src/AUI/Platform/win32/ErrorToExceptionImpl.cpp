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

    AString message(messageBuffer, size);

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
        case ERROR_ALREADY_EXISTS:
            break;
        default:
            throw AIOException("{} (code {})"_format(message, lastErrorCode));
    }
}
