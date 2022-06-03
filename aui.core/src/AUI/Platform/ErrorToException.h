#pragma once

#include <AUI/Common/AString.h>

#ifdef unix
#undef unix
#endif

namespace aui::impl {
    void lastErrorToException(AString message);

    namespace unix {
        void lastErrorToException(AString message); // unix errors are suitable under windows
    }
}

