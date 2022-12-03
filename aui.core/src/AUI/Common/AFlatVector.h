#pragma once

#include <AUI/Common/AVector.h>
#include <utility>


/**
 * @brief Stack-only vector implementation.
 * @tparam T
 * @tparam COUNT
 */
template<typename T, std::size_t MAX_SIZE>
class AFlatVector {
public:

    ~AFlatVector() {
        destructAll();
    }

    std::size_t size() const noexcept {
        return mSize;
    }

    T& operator[](std::size_t index) noexcept {
        assert(("index out of bounds", index < size()));
        return data()[index];
    }

    const T& operator[](std::size_t index) const noexcept {
        assert(("index out of bounds", index < size()));
        return data()[index];
    }

    [[nodiscard]]
    T& at(std::size_t index) {
        if (index >= size()) {
            aui::impl::outOfBoundsException();
        }
        return operator[](index);
    }

    [[nodiscard]]
    const T& at(std::size_t index) const {
        if (index >= size()) {
            aui::impl::outOfBoundsException();
        }
        return operator[](index);
    }

    T* data() noexcept {
        return reinterpret_cast<T*>(&mStorage);
    }

    const T* data() const noexcept {
        return reinterpret_cast<const T*>(&mStorage);
    }

    void clear() {
        destructAll();
        mSize = 0;
    }

    void push_back(T value) {
        assert(("size exceeded", mSize < MAX_SIZE));
        new (data() + mSize) T(std::move(value));
    }

    AFlatVector& operator<<(T value) {
        push_back(std::move(value));
        return *this;
    }



private:
    std::size_t mSize = 0;
    std::aligned_storage_t<sizeof(T) * MAX_SIZE, alignof(T)> mStorage;

    void destructAll() {
        for (size_t i = 0; i < size(); ++i) {
            data()[i].~T();
        }
    }
};