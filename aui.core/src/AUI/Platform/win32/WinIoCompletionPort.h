#pragma once


#include <Windows.h>
#include "AUI/Traits/values.h"


class WinIoCompletionPort: public aui::noncopyable {
private:
    HANDLE mCompletionPortHandle;
    ULONG_PTR mKey;

    static ULONG_PTR nextKey() noexcept;

public:
    WinIoCompletionPort() noexcept: mCompletionPortHandle(nullptr) {}
    ~WinIoCompletionPort();

    void create(HANDLE fileHandle);
};


