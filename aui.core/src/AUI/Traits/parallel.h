#pragma once


#include <AUI/Thread/AThreadPool.h>

namespace aui {
    template<typename Iterator, typename Functor>
    auto parallel(Iterator begin, Iterator end, Functor&& functor) {
        return AThreadPool::global().parallel(begin, end, std::forward<Functor>(functor));
    }
}