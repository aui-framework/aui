#pragma once

#include <AUI/Common/AString.h>

#ifdef unix
#undef unix
#endif

namespace aui::impl {
    struct Error {
        int nativeCode;
        AString description;
    };

    void lastErrorToException(AString message);
    Error lastError();

    namespace unix {
        void lastErrorToException(AString message); // unix errors are suitable under windows
        Error lastError();
    }
}

