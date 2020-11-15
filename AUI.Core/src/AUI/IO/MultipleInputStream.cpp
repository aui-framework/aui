//
// Created by alex2 on 13.11.2020.
//

#include "MultipleInputStream.h"

int MultipleInputStream::read(char* dst, int size) {
    while (!mInputStreams.empty()) {
        int r = mInputStreams.first()->read(dst, size);
        if (r <= 0) {
            mInputStreams.pop_front();
        } else {
            return r;
        }
    }
    return 0;
}
