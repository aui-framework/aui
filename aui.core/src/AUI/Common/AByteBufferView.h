#pragma once

#include <AUI/Traits/serializable.h>

/**
 * Acts like std::string_view but for AByteBuffer.
 * @note don't use const reference of the object. Passing by value allows compiler to use only registers.
 */
class AByteBufferView {
private:
    const char* mBuffer;
    size_t mSize;

public:
    AByteBufferView(): mBuffer(nullptr), mSize(0) {}
    AByteBufferView(const char* buffer, size_t size) : mBuffer(buffer), mSize(size) {}

    const char* data() const {
        return mBuffer;
    }

    size_t size() const {
        return mSize;
    }

    auto begin() const {
        return data();
    }
    auto end() const {
        return data() + size();
    }
};

template<>
struct ASerializable<AByteBufferView> {
    static void write(IOutputStream& os, AByteBufferView view) {
        os.write(view.data(), view.size());
    }
};