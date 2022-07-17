/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Thread/AThreadPool.h>
#include <AUI/Thread/AFuture.h>
#include <type_traits>

/**
 * @brief Passes the current class and type of the current class separated by comma. It's convenient to use with the
 *        connect function:
 * @ingroup useful_macros
 * @details
 * <table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       connect(clicked, this, &MyObject::handleClicked);
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       connect(clicked, me::handleClicked);
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 *
 */
#define me this, &std::remove_reference_t<decltype(*this)>

namespace aui::impl::slot {
    template<typename T>
    struct decode_type {
        using type = T;
    };

    template<typename T>
    struct decode_type<_<T>> {
        using type = T;
    };

    template<typename T>
    using decode_type_t = typename decode_type<T>::type;
}

/**
 * @brief Passes some variable and type of the variable separated by comma. It's convenient to use with the connect
 *        function (see examples).
 * @ingroup useful_macros
 * @details
 * <table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       connect(clicked, myObject, &MyObject::handleClicked);
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       connect(clicked, slot(myObject)::handleClicked);
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 */
#define slot(v) v, &aui::impl::slot::decode_type_t<std::decay_t<decltype(v)>>

/**
 * @brief Performs multiple operations on a single object without repeating its name.
 * @ingroup useful_macros
 * @param object object to perform operations on
 * @param lambda code executed in the context of an object (as its member function)
 * @details
 * @note It's an analogue to <code>with</code>, <code>apply</code> in Kotlin.
 *
 *
 *<table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       class Worker {
 *       public:
 *         void buildHouse();
 *         void plantTree();
 *         void raiseSon();
 *       };
 *       ...
 *       auto worker = _new<Worker>();
 *       worker->buildHouse();
 *       worker->plantTree();
 *       worker->raiseSon();
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       class Worker {
 *       public:
 *         void buildHouse();
 *         void plantTree();
 *         void raiseSon();
 *       };
 *       ...
 *       auto worker = _new<Worker>();
 *       perform_as_member(worker, {
 *         buildHouse();
 *         plantTree();
 *         raiseSon();
 *       });
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 */
#define perform_as_member(object, lambda)                                                  \
    struct __apply ## __FUNCTION__ ## __LINE__   : std::decay_t<decltype(object)>::stored_t { \
        void operator()() {                                                    \
            lambda;                                                            \
        }                                                                      \
    };                                                                         \
    (*reinterpret_cast<__apply ## __FUNCTION__ ## __LINE__ *>(object.get()))()

/**
 * @brief Performs multiple operations on a single object without repeating its name (in place)
 *        This function can be used as an operator on object.
 * @ingroup useful_macros
 * @param T object type to perform operations on
 * @param lambda code executed in the context of an object (as its member function)
 * @note analogue to <code>with</code>, <code>apply</code> in Kotlin
 * @details
 * `let` allows to call methods of newly created objects right in place. For example:
 *
 * <table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       auto tf = _new<ATextField>();
 *       tf->setText("Hello!");
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       auto tf = _new<ATextField>() let { it->setText("Hello!"); };
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 *
 * It's especially useful when building user interfaces:
 *
 * <table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       auto tf1 = _new<ATextField>();
 *       tf1->setText("Hello!");
 *       auto tf2 = _new<ATextField>();
 *       tf2->setText("World!");
 *       ...
 *       setContents(Vertical { // confusing
 *           tf1,
 *           tf2,
 *       });
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       setContents(Vertical { // clean, less code and easy to understand
 *           _new<ATextField>() let { it->setText("Hello!") },
 *           _new<ATextField>() let { it->setText("World!") },
 *       });
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 */
#define let ^ [&](const auto& it)

/**
 * @brief Allows to define a style to the view right in place.
 * @ingroup useful_macros
 * @details
 * @code{cpp}
 * #include <AUI/ASS/ASS.h>
 * using namespace ass;
 * ...
 * setContents(Centered {
 *   _new<ALabel>("Red text!") with_style { TextColor { AColor::RED } },
 * });
 * @endcode
 */
#define with_style + RuleWithoutSelector

/**
 * @brief Executes following {} block asynchronously in the @ref AThreadPool::global() "global" thread pool. Unlike
 * asyncX, does now allow to set lambda's capture. Lambda's capture is `[=]`.
 *
 * @ingroup useful_macros
 * @return <code>AFuture<T></code> where <code>T</code> is the return type of the lambda.
 * @note When <code>AFuture<T></code> is destroyed, the corresponding `async` task is either cancelled or removed from
 * the execution queue. Use AFutureSet or AAsyncHolder to keep multiple AFuture<T> alive.
 *
 * @details
 * <p>Example without a return value</p>
 * @code{cpp}
 * auto task = async {
 *   AThread::sleep(1000); // a long task
 * };
 * @endcode
 * <p>Example with a return value</p>
 * @code{cpp}
 * auto futureStatus = async {
 *   int status;
 *   ...
 *   AThread::sleep(1000); // a long task
 *   ...
 *   return status;
 * };
 * int status = *futureStatus;
 * @endcode
 *
 * Lambda operators are supported:
 * @code{cpp}
 * auto futureStatus = async mutable noexcept {
 *   int status;
 *   ...
 *   AThread::sleep(1000); // a long task
 *   ...
 *   return status;
 * };
 * int status = *futureStatus;
 * @endcode
 */
#define async AThreadPool::global() * [=]()


/**
 * @brief Executes following {} block asynchronously in the @ref AThreadPool::global() "global" thread pool. Unlike
 * async, allows to set lambda's capture but you should always specify lambda's capture.
 *
 * @ingroup useful_macros
 * @return <code>AFuture<T></code> where <code>T</code> is the return type of the lambda.
 * @note When <code>AFuture<T></code> is destroyed, the corresponding `async` task is either cancelled or removed from
 * the execution queue. Use AFutureSet or AAsyncHolder to keep multiple AFuture<T> alive.
 *
 * @details
 * <p>Example without a return value</p>
 * @code{cpp}
 * auto task = asyncX [&] {
 *   AThread::sleep(1000); // a long task
 * };
 * @endcode
 * <p>Example with a return value</p>
 * @code{cpp}
 * auto futureStatus = asyncX [&] {
 *   int status;
 *   ...
 *   AThread::sleep(1000); // a long task
 *   ...
 *   return status;
 * };
 * int status = *futureStatus;
 * @endcode
 */
#define asyncX AThreadPool::global() *

/**
 * @brief Executes following function call or {} block once per program execution
 * @ingroup useful_macros
 * @details Guarantees that the following code block will be executed only once per application execution.
 * <table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       static bool done = false;
 *       if (!done) {
 *           done = true;
 *           std::printf("Only once!");
 *       }
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       do_once {
 *           std::printf("Only once!");
 *       }
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 */
#define do_once if(static uint8_t _aui_once = 0; !_aui_once++)

/**
 * @brief Executes lambda on current object's thread.
 */
#define ui_thread (*getThread()) * [=]()

/**
 * @brief Executes lambda on current object's thread. Allows to determine lambda's capture.
 */
#define ui_threadX (*getThread()) *
#define repeat(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex)
#define repeat_async(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex) AThreadPool::global() << [=]()