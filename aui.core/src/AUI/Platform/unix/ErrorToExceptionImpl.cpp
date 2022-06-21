//
// Created by Alex2772 on 5/18/2022.
//

#include <AUI/IO/AIOException.h>
#include <AUI/Platform/ErrorToException.h>


void aui::impl::lastErrorToException(AString message) {
    aui::impl::unix::lastErrorToException(std::move(message));
}

aui::impl::Error aui::impl::lastError() {
    return aui::impl::unix::lastError();
}
