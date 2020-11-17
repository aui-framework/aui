//
// Created by alex2 on 16.11.2020.
//

#include "ALimitedInputStream.h"

int ALimitedInputStream::read(char* dst, int size) {
    int toRead = glm::min(mLimit, size_t(size));
    if (toRead) {
        mLimit -= toRead;
        int read = mInputStream->read(dst, toRead);
        assert(read <= toRead);
        return read;
    }
    return 0;
}
