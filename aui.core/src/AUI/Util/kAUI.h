// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
 *       AUI_PERFORM_AS_MEMBER(*worker, {
 *         buildHouse();
 *         plantTree();
 *         raiseSon();
 *       });
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 */
#define AUI_PERFORM_AS_MEMBER(object, lambda)                                                  \
    struct __apply ## __FUNCTION__ ## __LINE__   : std::decay_t<decltype(object)> { \
        void operator()() {                                                    \
            lambda;                                                            \
        }                                                                      \
    };                                                                         \
    (static_cast<__apply ## __FUNCTION__ ## __LINE__ &>(object))()

/**
 * @brief Emits a signal of a foreign object.
 * @details
 * <table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       class SomeObject {
 *       public:
 *         emits<> someSignal;
 *       };
 *       ...
 *       auto obj = _new<SomeObject>();
 *       (*obj) ^ obj->someSignal();
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       class SomeObject {
 *       public:
 *         emits<> someSignal;
 *       };
 *       ...
 *       auto obj = _new<SomeObject>();
 *       AUI_EMIT_FOREIGN_SIGNAL(obj)->someSignal();
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 */
#define AUI_EMIT_FOREIGN_SIGNAL(object) (*object) ^ object


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
#define do_once if(static bool _aui_once = false; (!_aui_once && (_aui_once = true)))

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