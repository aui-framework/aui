#pragma once


class AByteBufferRef {
private:
    const char* mBuffer;
    size_t mSize;

public:
    AByteBufferRef(): mBuffer(nullptr), mSize(0) {}
    AByteBufferRef(const char* buffer, size_t size) : mBuffer(buffer), mSize(size) {}

    const char* data() const {
        return mBuffer;
    }
    size_t size() const {
        return mSize;
    }
};