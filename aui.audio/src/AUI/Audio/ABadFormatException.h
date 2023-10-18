#pragma once

#include "AUI/Common/AException.h"

namespace aui::audio {
    class ABadFormatException : public AException {
    public:
        using AException::AException;
    };
}
