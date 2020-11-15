//
// Created by alex2 on 18.09.2020.
//

#include "ByteBufferOutputStream.h"

int ByteBufferOutputStream::write(const char* src, int size) {
    mBuffer->put(src, size);
    return size;
}
