#pragma once


#include <cstdint>
#include <cstddef>
#include <cassert>
#include <AUI/Common/AVector.h>

template<typename T>
class AArrayView {
public:
    AArrayView(const T* data, std::size_t count) noexcept : mData(data), mCount(count) {}
    AArrayView(const AVector<T>& vector) noexcept: mData(vector.data()), mCount(vector.size()) {

    }

    template<std::size_t N>
    AArrayView(const T (&rawArray)[N]) noexcept: mData(rawArray), mCount(N) {}

    template<std::size_t N>
    AArrayView(const std::array<T, N>& array) noexcept: mData(array.data()), mCount(N) {}

    [[nodiscard]]
    const T* data() const noexcept {
        return mData;
    }

    [[nodiscard]]
    size_t size() const noexcept {
        return mCount;
    }
    [[nodiscard]]
    size_t sizeInBytes() const noexcept {
        return mCount * sizeof(T);
    }

    const T& operator[](std::size_t index) const noexcept {
        assert(("out of bounds", index < mCount));
        return mData[index];
    }

    const T* begin() const noexcept {
        return mData;
    }

    const T* end() const noexcept {
        return mData + mCount;
    }

private:
    const T* mData;
    std::size_t mCount;
};