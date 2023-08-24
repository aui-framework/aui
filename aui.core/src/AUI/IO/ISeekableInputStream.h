#pragma once

#include "IInputStream.h"

class ISeekableInputStream: public IInputStream {
public:
    virtual ~ISeekableInputStream() = default;
    virtual void seek(std::streamoff offset, std::ios::seekdir seekDir) = 0;
    virtual size_t tell() = 0;
    virtual bool isEof() = 0;

    size_t fileSize() {
        auto current = tell();
        seek(0, std::ios::end);
        auto size = tell();
        seek(current, std::ios::beg);
        return size;
    }
};