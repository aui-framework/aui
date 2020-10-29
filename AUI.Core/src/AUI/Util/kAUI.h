#pragma once

#include <AUI/Thread/AThreadPool.h>

/**
 * \brief Выполнить несколько операций над одним объектом, не повторяя его имени.
 * \param object объект, над которым нужно провести операции
 * \param lambda код операции, выполняющийся в контексте объекта (как его member-функция)
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
    struct __apply ## __FUNCTION__ ## __LINE__   : std::decay_t<decltype(object)>::stored_t { \
        void operator()() {                                                    \
            ([&]() lambda )();                                                 \
        }                                                                      \
    };                                                                         \
    (*reinterpret_cast<__apply ## __FUNCTION__ ## __LINE__ *>(object.get()))()

/**
 * \brief Выполнить несколько операций над одним объектом, не повторяя его имени (in place).
 * \param T тип объекта, над которым нужно провести операции
 * \param lambda код операции, выполняющийся в контексте объекта (как его member-функция)
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
 * auto worker = _new&lt;Worker&gt;() let (Worker, {<br />
 * &#09;buildHouse();<br />
 * &#09;plantTree();<br />
 * &#09;raiseSon();<br />
 * });<br />
 * </code>
 */
#define let(T, lambda)               \
    .applyOnFunctor([&](const _<T>& object) { \
        apply(object, lambda);   \
    })

/**
 * \brief Выполнить следующий блок асинхронно, т. е. в стандартном тредпуле. В отличие от <code>async</code> <b>не</b>
 *        позволяет задать область видимости лямбды.
 * \return ничего, если в блоке нет ключевого слова <code>return</code>; <code>_&lt;AFuture&lt;T&gt;&gt;</code>, если
 *         блок имеет ключевое слово <code>return</code>, где <code>T</code> - тип возвращаемого объекта.
 * \see asyncX
 * \example
 * <p>Пример без возвращаемого значения</p>
 * <code>
 * async {<br />
 * &#09;...<br />
 * };<br />
 * </code>
 * <p>Пример с возвращаемым значением</p>
 * <code>
 * auto futureStatus = async {<br />
 * &#09;int status;<br />
 * &#09;...<br />
 * &#09;return status;<br />
 * };<br />
 * int status = **futureStatus;<br />
 * </code>
 */
#define async AThreadPool::global() * [=]()


/**
 * \brief Выполнить следующий блок асинхронно, т. е. в стандартном тредпуле. В отличие от <code>async</code> позволяет
 *        задать область видимости лямбды.
 * \return ничего, если в блоке нет ключевого слова <code>return</code>; <code>_&lt;AFuture&lt;T&gt;&gt;</code>, если
 *         блок имеет ключевое слово <code>return</code>, где <code>T</code> - тип возвращаемого объекта.
 * \see async
 * \example
 * <p>Пример без возвращаемого значения</p>
 * <code>
 * asyncX [&]() {<br />
 * &#09;...<br />
 * };<br />
 * </code>
 * <p>Пример с возвращаемым значением</p>
 * <code>
 * auto futureStatus = asyncX [&]() {<br />
 * &#09;int status;<br />
 * &#09;...<br />
 * &#09;return status;<br />
 * };<br />
 * int status = **futureStatus;<br />
 * </code>
 */
#define asyncX AThreadPool::global() *
#define ui (*getThread()) * [=]()
#define uiX (*getThread()) *
#define repeat(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex)
#define repeat_async(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex) AThreadPool::global() << [=]()