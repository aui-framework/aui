//
// Created by Alex2772 on 5/18/2022.
//

#include <Windows.h>
#include <AUI/IO/AIOException.h>
#include <AUI/Platform/ErrorToException.h>

aui::impl::Error aui::impl::lastError() {
    int errorCode = GetLastError();
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
void aui::impl::lastErrorToException(AString message) {
    auto[lastErrorCode, description] = lastError();
    message += ": ";
    message += description;
    switch (lastErrorCode) {
        case ERROR_FILE_NOT_FOUND:
            throw AFileNotFoundException(message);
        case ERROR_ACCESS_DENIED:
            throw AAccessDeniedException(message);
        case ERROR_ALREADY_EXISTS:
            break;
        default:
            throw AIOException(message);
    }
}
