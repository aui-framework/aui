//
// Created by Alex2772 on 4/18/2022.
//

#include "WinIoCompletionPort.h"
#include "AUI/Common/AException.h"
#include <atomic>

void WinIoCompletionPort::create(HANDLE fileHandle) {
    mKey = nextKey();
    if (!(mCompletionPortHandle = CreateIoCompletionPort(fileHandle, nullptr, mKey, 0))) {
        throw AException("CreateIoCompletionPort failed");
    }
}

WinIoCompletionPort::~WinIoCompletionPort() {
    if (mCompletionPortHandle) {
        CloseHandle(mCompletionPortHandle);
    }
}

ULONG_PTR WinIoCompletionPort::nextKey() noexcept {
    static std::atomic_uintptr_t key = 0;
    return key++;
}
