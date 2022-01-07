#pragma once

#include "AFuture.h"

template<typename T>
class AFutureSet: public AVector<_<AFuture<T>>> {
public:
    using AVector<_<AFuture<T>>>::AVector;

    void waitForAll() {
        for (const _<AFuture<T>>& v : *this) {
            v->operator*();
        }
    }
};


