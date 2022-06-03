#pragma once

#include <AUI/Common/AString.h>

namespace aui::impl {
    void lastErrorToException(AString message);

    namespace unix {
        void lastErrorToException(AString message); // unix errors are suitable under windows
    }
}

