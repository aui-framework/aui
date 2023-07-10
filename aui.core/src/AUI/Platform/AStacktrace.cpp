//
// Created by Alex2772 on 7/10/2023.
//

#include "AStacktrace.h"

aui::range<AStacktrace::iterator> AStacktrace::stripBeforeFunctionCall(void* pFunction, int maxAllowedOffsetInBytes) {
    auto it = std::find_if(begin(), end(), [&](const Entry& e) {
        return (reinterpret_cast<std::uintptr_t>(e.ptr()) - reinterpret_cast<std::uintptr_t>(pFunction)) <= maxAllowedOffsetInBytes;
    });
    if (it != end()) {
        return { it + 1, end() };
    }
    return *this;
}