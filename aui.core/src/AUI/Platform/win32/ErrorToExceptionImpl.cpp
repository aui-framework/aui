//
// Created by Alex2772 on 5/18/2022.
//

#include <Windows.h>
#include <AUI/IO/AIOException.h>
#include <AUI/Platform/ErrorToException.h>

static AString errorMessage(DWORD errorMessageID) noexcept {
    if(errorMessageID == 0)
        return {}; //No error message has been recorded

    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

    AString message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}
void aui::impl::lastErrorToException(AString message) {
    auto lastError = GetLastError();
    message += ": ";
    message += errorMessage(lastError);
    switch (lastError) {
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
