//
// Created by alex2772 on 7/4/21.
//

#pragma once

#include <AUI/Common/AString.h>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

namespace AReflect {
    template<typename T>
    AString name(T* v) {
#ifdef _MSVC_VER
    return typeid(*v).name();
#else
    char buf[0x100];
    size_t length;
    int status;
    abi::__cxa_demangle(typeid(*v).name(), buf, &length, &status);
    return buf;
#endif
    }
}