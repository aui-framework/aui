#pragma once

#include <AUI/Thread/AThreadPool.h>

/**
 * \brief Выполнить несколько операций над одним объектом, не повторяя его имени.
 * \note аналог <code>with</code>, <code>apply</code> в Kotlin
 * \example
 * <code>
 * class Worker {<br />
 * public:<br />
 * &#09;void buildHouse();<br />
 * &#09;void plantTree();<br />
 * &#09;void raiseSon();<br />
 * };<br />
 * ...<br />
 * auto worker = _new&lt;Worker&gt;();<br />
 * apply(worker, {<br />
 * &#09;buildHouse();<br />
 * &#09;plantTree();<br />
 * &#09;raiseSon();<br />
 * });<br />
 * </code>
 */
#define apply(object, lambda)                                                  \
    struct __apply ## __FUNCTION__ ## __LINE__   : decltype(object)::stored_t { \
        void operator()() {                                                    \
            ([&]() lambda )();                                                 \
        }                                                                      \
    };                                                                         \
    (*reinterpret_cast<__apply ## __FUNCTION__ ## __LINE__ *>(object.get()))()

#define by(T, lambda)               \
    .applyOnFunctor([&](_<T> object) { \
        apply(object, lambda);   \
    })

#define async AThreadPool::global() * [=]()
#define ui (*getThread()) * [=]()
#define asyncX AThreadPool::global() *
#define uiX (*getThread()) *
#define repeat(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex)
#define repeat_async(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex) AThreadPool::global() << [=]()