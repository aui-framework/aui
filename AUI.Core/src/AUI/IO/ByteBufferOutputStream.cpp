//
// Created by alex2 on 18.09.2020.
//

#include "ByteBufferOutputStream.h"

int ByteBufferOutputStream::write(const char* dst, int size) {
    mBuffer->put(dst, size);
    return size;
}
