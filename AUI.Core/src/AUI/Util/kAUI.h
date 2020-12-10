#pragma once

#include <AUI/Thread/AThreadPool.h>
#include <type_traits>

/**
 * \brief Получить текущий класс и тип текущего класса. Удобно использовать в связке с функцией connect:
 * \example
 * <code>
 * class MyObject: public AView {<br />
 * private:<br />
 * &#09;void handleClicked() { ... }<br />
 * <br />
 * <br />
 * public:<br />
 * &#09;Class() {<br />
 * &#09;&#09;connect(clicked, me::handleClicked);<br />
  *&#09;}<br />
 * };
 * </code>
 */
#define me this, &std::remove_reference_t<decltype(*this)>

/**
 * \brief Получить AObject и тип этого AObject. Удобно использовать в связке с функцией connect:
 * \example
 * <code>
 * class MyObject: public AView {<br />
 * private:<br />
 * &#09;void handleClicked() { ... }<br />
 * };<br />
 * ...<br />
 * SomeOtherClass::SomeOtherClass() {<br />
 * &#09;auto myObject = _new<MyObject>();<br />
 * &#09;connect(clicked, slot(myObject)::handleClicked);<br />
  *}
 * </code>
 */
#define slot(v) v, &decltype(v)::stored_t

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

/**
 * \brief Выполнить следующий блок один раз за один запуск программы.
 * \example
 * <p>Пример с фигурными скобками</p>
 * <code>
 * do_once {<br />
 * &#09;aui::importPlugin("MyPlugin");<br />
 * }<br />
 * </code>
 * <p>Пример без фигурных скобок</p>
 * <code>
 * do_once aui::importPlugin("MyPlugin");
 * </code>
 */
#define do_once static uint8_t _aui_once = 0; if(!_aui_once++)

#define ui (*getThread()) * [=]()
#define uiX (*getThread()) *
#define repeat(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex)
#define repeat_async(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex) AThreadPool::global() << [=]()