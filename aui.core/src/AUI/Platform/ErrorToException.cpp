//
// Created by Alex2772 on 5/18/2022.
//

#include <AUI/IO/AIOException.h>
#include <AUI/Platform/ErrorToException.h>


void aui::impl::unix::lastErrorToException(AString message) {
    auto lastError = errno;
    message += ": ";
    message += strerror(lastError);
    switch (lastError) {
        case ENOENT:
            throw AFileNotFoundException(message);
        case EPERM:
        case EACCES:
            throw AAccessDeniedException(message);
        case EEXIST:
            break;
        default:
            throw AIOException(message);
    }
}
