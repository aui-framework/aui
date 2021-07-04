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
#ifdef _MSC_VER
    return typeid(*v).name();
#else
    int status;
    auto mangledName = typeid(*v).name();
    auto ptr = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);
    AString result = ptr;
    free(ptr);
    return result;
#endif
    }
}