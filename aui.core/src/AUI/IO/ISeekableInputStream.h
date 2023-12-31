#pragma once

#include "IInputStream.h"

/**
 * @brief Represents an input stream with updating reading position.
 * @ingroup io
 */
class ISeekableInputStream: public IInputStream {
public:
    virtual ~ISeekableInputStream() = default;

    /**
     * @brief change reading position, a way of changing depends on seekDir parameter
     */
    virtual void seek(std::streamoff offset, std::ios::seekdir seekDir) = 0;

    /**
     * @brief return current reading position
     * @return current reading pos
     */
    virtual size_t tell() = 0;

    /**
     * @brief returns true if end of stream has been reached
     * @return true if end of stream has been reached
     */
    virtual bool isEof() = 0;

    /**
     * @brief return size of the stream
     * @return size of the stream
     */
    size_t fileSize() {
        auto current = tell();
        seek(0, std::ios::end);
        auto size = tell();
        seek(current, std::ios::beg);
        return size;
    }
};