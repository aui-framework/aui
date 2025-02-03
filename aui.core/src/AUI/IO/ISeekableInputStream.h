#pragma once

#include "IInputStream.h"

/**
 * @brief Represents an input stream with updating reading position.
 * @ingroup io
 */
class ISeekableInputStream: public IInputStream {
public:
    ~ISeekableInputStream() override = default;

    enum class Seek {
        /**
         * Seek relatively to the begin of file
         */
        BEGIN,

        /**
         * Seek relatively to the current position
         */
        CURRENT,

        /**
         * Seek relative to the end of file
         */
        END
    };

    /**
     * @brief change reading position, a way of changing depends on seekDir parameter
     */
    virtual void seek(std::streamoff offset, Seek seekDir) = 0;

    /**
     * @brief return current reading position
     * @return current reading pos
     */
    [[nodiscard]] virtual std::streampos tell() noexcept = 0;

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
        seek(0, Seek::END);
        auto size = tell();
        seek(current, Seek::BEGIN);
        return size;
    }
};