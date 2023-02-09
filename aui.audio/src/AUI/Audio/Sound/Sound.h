#pragma once

#include <utility>
#include <vector>
#include "AUI/Audio/Stream/IStream.h"
#include "AUI/Audio/AAudioFormat.h"

namespace Audio {
class SoundStream: public IStream {
    public:
        virtual AAudioFormat info() = 0;

        /**
         * Промотать до начала.
         * @return true, если получилось
         */
        virtual void rewind() = 0;

        virtual ~SoundStream() = default;
    };
}