#pragma once

#include <AUI/Common/AObject.h>

template<typename Callback>
class ARaiiHelper {
public:
    ARaiiHelper(Callback&& callback) noexcept: mCallback(std::forward<Callback>(callback)) {

    }
    ~ARaiiHelper() {
        mCallback();
    }

private:
    std::decay_t<Callback> mCallback;
};