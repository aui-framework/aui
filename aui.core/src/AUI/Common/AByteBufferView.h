#pragma once

#include <AUI/Traits/serializable.h>

/**
 * @brief Acts like std::string_view but for AByteBuffer.
 * @ingroup core
 * @note don't use const reference of AByteBufferView. Passing by const reference forces compiler to use memory instead
 * of registers.
 */
class API_AUI_CORE AByteBufferView {
private:
    const char* mBuffer;
    size_t mSize;

public:
    AByteBufferView() noexcept: mBuffer(nullptr), mSize(0) {}
    AByteBufferView(const char* buffer, size_t size) noexcept: mBuffer(buffer), mSize(size) {}
    explicit AByteBufferView(const std::string& string) noexcept: mBuffer(string.data()), mSize(string.size()) {}
    explicit AByteBufferView(std::string_view string) noexcept: mBuffer(string.data()), mSize(string.size()) {}

    [[nodiscard]]
    AByteBufferView slice(std::size_t offset) const noexcept {
        return slice(offset, size() - offset);
    }

    [[nodiscard]]
    AByteBufferView slice(std::size_t offset, std::size_t size) const noexcept {
        assert(("out of bounds", offset + size <= mSize));
        return { mBuffer + offset, size };
    }

    [[nodiscard]]
    const char* data() const noexcept {
        return mBuffer;
    }

    [[nodiscard]]
    size_t size() const noexcept {
        return mSize;
    }

    [[nodiscard]]
    auto begin() const noexcept {
        return data();
    }
    [[nodiscard]]
    auto end() const noexcept {
        return data() + size();
    }

    [[nodiscard]]
    AString toHexString() const;

    [[nodiscard]]
    AString toBase64String() const;

    template<typename T>
    [[nodiscard]]
    const T& as() const {
        if (mSize != sizeof(T)) {
            throw AException("as<T>(): invalid size");
        }
        return *reinterpret_cast<const T*>(mBuffer);
    }

    template<typename T>
    [[nodiscard]]
    static AByteBufferView fromRaw(const T& data) noexcept {
        return { reinterpret_cast<const char*>(&data), sizeof(data) };
    }
};

template<>
struct ASerializable<AByteBufferView> {
    static void write(IOutputStream& os, AByteBufferView view) {
        os.write(view.data(), view.size());
    }
};