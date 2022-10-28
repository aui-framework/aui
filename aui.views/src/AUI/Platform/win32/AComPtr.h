#pragma once


#include <cassert>

/**
 * @brief Takes care of AddRef and Release() reference counting mechanism of COM objects.
 * @tparam T
 */
template<typename T>
class AComPtr {
public:

    AComPtr() {
        mValue = nullptr;
    }

    ~AComPtr() {
        if (mValue) {
            mValue->Release();
        }
    }

    T** operator&() noexcept {
        assert(("value already set", mValue == nullptr));
        return &mValue;
    }

    AComPtr(T* value): mValue(value) {}
    AComPtr(const AComPtr<T>& rhs): mValue(rhs.mValue) {
        if (mValue) {
            mValue->AddRef();
        }
    }

    AComPtr(AComPtr<T>&& rhs): mValue(rhs.mValue) {
        rhs.mValue = nullptr;
    }

    [[nodiscard]]
    T* value() const noexcept {
        assert(mValue != nullptr);
        return mValue;
    }

    [[nodiscard]]
    T* operator*() const noexcept {
        return value();
    }

    [[nodiscard]]
    T* operator->() const noexcept {
        return value();
    }


    [[nodiscard]]
    operator T*() const noexcept {
        return value();
    }

    [[nodiscard]]
    operator bool() const noexcept {
        return mValue != nullptr;
    }

private:
    T* mValue;
};


