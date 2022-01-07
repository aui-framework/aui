//
// Created by Alex2772 on 1/6/2022.
//

#include "ARandomInputStream.h"

size_t ARandomInputStream::read(char* dst, size_t size) {
    std::uniform_int_distribution<int> in;
    for (int i = 0; i < size; ++i) {
        dst[i] = in(mRandomEngine);
    }
    return size;
}
