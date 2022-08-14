//
// Created by alex2772 on 4/19/22.
//

#pragma once


#include "AUI/Traits/values.h"

class UnixEventFd: public aui::noncopyable {
public:
    UnixEventFd() noexcept;
    ~UnixEventFd();

    void set() noexcept;
    void reset() noexcept;

    [[nodiscard]]
    int handle() noexcept {
        return mIn;
    }

private:
#if AUI_PLATFORM_APPLE
    int mOut, mIn;
#else
    int mHandle;
#endif
};



