#pragma once

#include <cstddef>
#include <AUI/IO/ISeekableInputStream.h>
#include <AUI/IO/IOutputStream.h>


namespace aui {
class ISeekableOutputStream: public IOutputStream {
public:
    ~ISeekableOutputStream() override = default;

    /**
     * @brief change reading position, a way of changing depends on seekDir parameter
     */
    virtual void seek(std::streamoff offset, ASeekDir seekDir) = 0;

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
        seek(0, ASeekDir::END);
        auto size = tell();
        seek(current, ASeekDir::BEGIN);
        return size;
    }
};
}
