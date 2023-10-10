#include "APortionedMultipleStream.h"

size_t APortionedMultipleStream::read(char *dst, size_t size) {
    while (!mDeque.empty()) {
        size_t readBytes = mDeque.front()->read(dst, size);
        if (readBytes != 0) {
            return readBytes;
        }

        mDeque.pop_front();
    }

    return 0;
}
