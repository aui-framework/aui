#pragma once

#include <Windows.h>

/**
 * @brief RAII helper to access HGLOBAL data.
 */
class ScopedHGlobal {
public:
    explicit ScopedHGlobal(HGLOBAL blob) noexcept: mBlob(blob) {
        mAccessibleData = static_cast<char*>(GlobalLock(blob));
    }

    ~ScopedHGlobal() {
        GlobalUnlock(mBlob);
    }

    [[nodiscard]]
    char* data() const noexcept {
        return mAccessibleData;
    }

    [[nodiscard]]
    std::size_t size() const noexcept {
        return GlobalSize(mBlob);
    }

    operator AByteBufferView() const noexcept {
        return { data(), size() };
    }

private:
    HGLOBAL mBlob;
    char* mAccessibleData;
};