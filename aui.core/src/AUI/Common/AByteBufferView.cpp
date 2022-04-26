//
// Created by Alex2772 on 4/18/2022.
//

#include "AByteBufferView.h"

AString AByteBufferView::toHexString() const {
    AString result;
    result.reserve(size() * 2 + 10);
    char buf[8];

    for (size_t i = 0; i < size(); ++i) {
        sprintf(buf, "%02x", static_cast<unsigned>(mBuffer[i]) & 0xff);
        result += buf;
    }
    return result;
}