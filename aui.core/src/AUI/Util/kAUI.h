/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <range/v3/range_fwd.hpp>
#include <AUI/Thread/AThreadPool.h>
#include <AUI/Thread/AFuture.h>
#include <type_traits>

/**
 * @brief Passes the current class and type of the current class separated by comma. It's convenient to use with the
 *        connect function:
 * @ingroup signal_slot
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
 * @ingroup signal_slot
 * @details
 * Quick example:
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
 *       connect(clicked, AUI_SLOT(myObject)::handleClicked);
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 *
 * @note If you are intended to reference this-> object, consider using @ref #me instead.
 */
#define AUI_SLOT(v) v, &aui::impl::slot::decode_type_t<std::decay_t<decltype(v)>>

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
 * @ingroup signal_slot
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
 * @brief Defers execution of the next block to the end of current block (RAII scope).
 * @ingroup useful_macros
 * @details
 * This example prints "Hello world\n":
 * @code{cpp}
 * AUI_DEFER { printf(" world\n") };
 * printf("Hello");
 * @endcode
 * @sa ARaiiHelper
 */
#define AUI_DEFER ARaiiHelper AUI_PP_CAT($AUI_DEFER_at_line_,  __LINE__) = [&]


/**
 * @brief Performs multiple operations on a single object without repeating its name (in place)
 *        This function can be used as an operator on object.
 * @ingroup useful_macros
 * @param T object type to perform operations on
 * @param lambda code executed in the context of an object (as its member function)
 * @note analogue to <code>with</code>, <code>apply</code> in Kotlin
 * @details
 * `AUI_LET` allows to call methods of newly created objects right in place. For example:
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
 *       auto tf = _new<ATextField>() AUI_LET { it->setText("Hello!"); };
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
 *       // clean, less code and easy to understand
 *       setContents(Vertical {
 *           _new<ATextField>() AUI_LET { it->setText("Hello!") },
 *           _new<ATextField>() AUI_LET { it->setText("World!") },
 *       });
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 */
#define AUI_LET ^ [&](const auto& it)

/**
 * @brief Allows to define a style to the view right in place.
 * @ingroup useful_macros
 * @details
 * @code{cpp}
 * #include <AUI/ASS/ASS.h>
 * using namespace ass;
 * ...
 * setContents(Centered {
 *   _new<ALabel>("Red text!") AUI_WITH_STYLE { TextColor { AColor::RED } },
 * });
 * @endcode
 *
 * Also applicable to declarative-style views:
 * @code{cpp}
 * #include <AUI/ASS/ASS.h>
 * using namespace ass;
 * ...
 * setContents(Centered {
 *   Label { "Red text!" } AUI_WITH_STYLE { TextColor { AColor::RED } },
 * });
 * @endcode
 */
#define AUI_WITH_STYLE & ass::PropertyListRecursive

/**
 * @brief Executes following {} block asynchronously in the @ref AThreadPool::global() "global" thread pool. Unlike
 * AUI_THREADPOOL_X, does now allow to set lambda's capture. Lambda's capture is `[=]`.
 *
 * @ingroup useful_macros
 * @return <code>AFuture<T></code> where <code>T</code> is the return type of the lambda.
 * @note When <code>AFuture<T></code> is destroyed, the corresponding `AUI_THREADPOOL` task is either cancelled or removed from
 * the execution queue. Use AFutureSet or AAsyncHolder to keep multiple AFuture<T> alive.
 *
 * @details
 * <p>Example without a return value</p>
 * @code{cpp}
 * auto task = AUI_THREADPOOL {
 *   AThread::sleep(1000); // a long task
 * };
 * @endcode
 * <p>Example with a return value</p>
 * @code{cpp}
 * auto futureStatus = AUI_THREADPOOL {
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
 * auto futureStatus = AUI_THREADPOOL mutable noexcept {
 *   int status;
 *   ...
 *   AThread::sleep(1000); // a long task
 *   ...
 *   return status;
 * };
 * int status = *futureStatus;
 * @endcode
 */
#define AUI_THREADPOOL AThreadPool::global() * [=, this]()


/**
 * @brief Executes following {} block asynchronously in the @ref AThreadPool::global() "global" thread pool. Unlike
 * AUI_THREADPOOL, allows to set lambda's capture but you should always specify lambda's capture.
 *
 * @ingroup useful_macros
 * @return <code>AFuture<T></code> where <code>T</code> is the return type of the lambda.
 * @note When <code>AFuture<T></code> is destroyed, the corresponding `AUI_THREADPOOL` task is either cancelled or removed from
 * the execution queue. Use AFutureSet or AAsyncHolder to keep multiple AFuture<T> alive.
 *
 * @details
 * <p>Example without a return value</p>
 * @code{cpp}
 * auto task = AUI_THREADPOOL_X [&] {
 *   AThread::sleep(1000); // a long task
 * };
 * @endcode
 * <p>Example with a return value</p>
 * @code{cpp}
 * auto futureStatus = AUI_THREADPOOL_X [&] {
 *   int status;
 *   ...
 *   AThread::sleep(1000); // a long task
 *   ...
 *   return status;
 * };
 * int status = *futureStatus;
 * @endcode
 */
#define AUI_THREADPOOL_X AThreadPool::global() *

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
 *       AUI_DO_ONCE {
 *           std::printf("Only once!");
 *       }
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 */
#define AUI_DO_ONCE if(static bool _aui_once = false; (!_aui_once && (_aui_once = true)))

/**
 * @brief Executes lambda on main thread.
 * @ingroup useful_macros
 */
#define AUI_UI_THREAD (*AThread::main()) * [=, this]()

/**
 * @brief Executes lambda on main thread. Allows to determine lambda's capture.
 * @ingroup useful_macros
 */
#define AUI_UI_THREAD_X (*AThread::main()) *

#define AUI_REPEAT(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex)
#define AUI_REPEAT_ASYNC(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex) AThreadPool::global() << [=]()

/**
 * @brief Executes the provided statements asynchronously on the global thread pool from contexts without this (free/static functions). Captures outer variables by value via a lambda.
 * @ingroup useful_macros
 */
#define AUI_THREADPOOL_SAFE(...)                        \
([&]() {                                                \
auto lambda = [=]() { __VA_ARGS__; };                   \
return AThreadPool::global() * lambda;                  \
}())

/**
 * @brief Same as AUI_THREADPOOL_SAFE, but for call sites that require an explicit return type in the lambda. Useful when the future’s value type can’t be deduced reliably.
 * @ingroup useful_macros
 */
#define AUI_THREADPOOL_SAFE_RET(Type, ...)              \
([&]() {                                                \
auto lambda = [=]() -> Type { __VA_ARGS__; };           \
return AThreadPool::global() * lambda;                  \
}())

/**
 * @brief Executes the provided statements asynchronously on the global thread pool with a noexcept lambda, suitable for functions that must not throw.
 * @ingroup useful_macros
 */
#define AUI_THREADPOOL_SAFE_NOEXCEPT(...)               \
([&]() {                                                \
auto lambda = [=]() noexcept { __VA_ARGS__; };          \
return AThreadPool::global() * lambda;                  \
}())

/**
 * @brief Combines explicit return type and noexcept for asynchronous execution on the global thread pool from contexts without this.
 * @ingroup useful_macros
 */
#define AUI_THREADPOOL_SAFE_NOEXCEPT_RET(Type, ...)     \
([&]() {                                                \
auto lambda = [=]() noexcept -> Type { __VA_ARGS__; };  \
return AThreadPool::global() * lambda;                  \
}())

/**
 * @brief Executes the provided statements on the main (UI) thread from contexts without this (free/static functions).
 *        Captures outer variables by value via a lambda.
 * @ingroup useful_macros
 */
#define AUI_UI_THREAD_SAFE(...)                         \
([&]() {                                                \
    auto lambda = [=]() { __VA_ARGS__; };               \
    return (*AThread::main()) * lambda;                 \
}())

/**
 * @brief Same as AUI_UI_THREAD_SAFE, but for call sites that require an explicit return type in the lambda.
 *        Useful when the future’s value type can’t be deduced reliably.
 * @ingroup useful_macros
 */
#define AUI_UI_THREAD_SAFE_RET(Type, ...)               \
([&]() {                                                \
    auto lambda = [=]() -> Type { __VA_ARGS__; };       \
    return (*AThread::main()) * lambda;                 \
}())

/**
 * @brief Executes the provided statements on the main (UI) thread with a noexcept lambda, suitable for functions that must not throw.
 * @ingroup useful_macros
 */
#define AUI_UI_THREAD_SAFE_NOEXCEPT(...)                \
([&]() {                                                \
    auto lambda = [=]() noexcept { __VA_ARGS__; };      \
    return (*AThread::main()) * lambda;                 \
}())

/**
 * @brief Combines explicit return type and noexcept for execution on the main (UI) thread from contexts without this.
 * @ingroup useful_macros
 */
#define AUI_UI_THREAD_SAFE_NOEXCEPT_RET(Type, ...)          \
([&]() {                                                    \
    auto lambda = [=]() noexcept -> Type { __VA_ARGS__; };  \
    return (*AThread::main()) * lambda;                     \
}())
