//
// Created by Alex2772 on 5/18/2022.
//

#include <AUI/IO/AIOException.h>
#include <AUI/Platform/ErrorToException.h>


void aui::impl::unix::lastErrorToException(AString message) {
    message += ": ";
    message += lastError().description;
    switch (errno) {
        case ENOENT:
            throw AFileNotFoundException(message);
        case EPERM:
        case EACCES:
            throw AAccessDeniedException(message);
        case EEXIST:
            break;
        case ENOSPC:
            throw ANoSpaceLeftException(message);
        default:
            throw AIOException(message);
    }
}

aui::impl::Error aui::impl::unix::lastError() {
    return { errno, strerror(errno) };
}