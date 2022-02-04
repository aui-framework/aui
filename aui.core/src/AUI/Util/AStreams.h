#pragma once

#include <AUI/Traits/serializable.h>

namespace AStreams {
    inline void copyAll(const IInputStream& is, IOutputStream& os, size_t bytes) {
        char buf[0x1000];
        while (bytes > 0) {
            size_t r = is.read(buf, (glm::min)(bytes, sizeof(buf)));
            if (r == 0) {
                throw AEOFException();
            }
            os.write(buf, r);
        }
    }
}