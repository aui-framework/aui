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

private:
    HGLOBAL mBlob;
    char* mAccessibleData;
};