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
 *       ```cpp
 *       connect(clicked, this, &MyObject::handleClicked);
 *       ```
 *     </td>
 *     <td>
 *       ```cpp
 *       connect(clicked, me::handleClicked);
 *       ```
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
 *       ```cpp
 *       connect(clicked, myObject, &MyObject::handleClicked);
 *       ```
 *     </td>
 *     <td>
 *       ```cpp
 *       connect(clicked, AUI_SLOT(myObject)::handleClicked);
 *       ```
 *     </td>
 *   </tr>
 * </table>
 *
 * If you are intended to reference this-> object, consider using [me] instead.
 */
#define AUI_SLOT(v) v, &aui::impl::slot::decode_type_t<std::decay_t<decltype(v)>>

/**
 * @brief Performs multiple operations on a single object without repeating its name.
 * @ingroup useful_macros
 * @param object object to perform operations on
 * @param lambda code executed in the context of an object (as its member function)
 * @details
 * It's an analogue to <code>with</code>, <code>apply</code> in Kotlin.
 *
 *
 *<table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       ```cpp
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
 *       ```
 *     </td>
 *     <td>
 *       ```cpp
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
 *       ```
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
 *       ```cpp
 *       class SomeObject {
 *       public:
 *         emits<> someSignal;
 *       };
 *       ...
 *       auto obj = _new<SomeObject>();
 *       (*obj) ^ obj->someSignal();
 *       ```
 *     </td>
 *     <td>
 *       ```cpp
 *       class SomeObject {
 *       public:
 *         emits<> someSignal;
 *       };
 *       ...
 *       auto obj = _new<SomeObject>();
 *       AUI_EMIT_FOREIGN_SIGNAL(obj)->someSignal();
 *       ```
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
 * ```cpp
 * AUI_DEFER { printf(" world\n") };
 * printf("Hello");
 * ```
 * @sa ARaiiHelper
 */
#define AUI_DEFER ARaiiHelper AUI_PP_CAT($AUI_DEFER_at_line_,  __LINE__) = [&]


/**
 * @brief Performs multiple operations on a single object without repeating its name (in place)
 *        This function can be used as an operator on object.
 * @ingroup useful_macros
 * @param T object type to perform operations on
 * @param lambda code executed in the context of an object (as its member function)
 * @details
 * Analogue to <code>with</code>, <code>apply</code> in Kotlin.
 *
 * `AUI_LET` allows to call methods of newly created objects right in place. For example:
 *
 * <table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       ```cpp
 *       auto tf = _new<ATextField>();
 *       tf->setText("Hello!");
 *       ```
 *     </td>
 *     <td>
 *       ```cpp
 *       auto tf = _new<ATextField>() AUI_LET { it->setText("Hello!"); };
 *       ```
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
 *       ```cpp
 *       auto tf1 = _new<ATextField>();
 *       tf1->setText("Hello!");
 *       auto tf2 = _new<ATextField>();
 *       tf2->setText("World!");
 *       ...
 *       setContents(Vertical { // confusing
 *           tf1,
 *           tf2,
 *       });
 *       ```
 *     </td>
 *     <td>
 *       ```cpp
 *       // clean, less code and easy to understand
 *       setContents(Vertical {
 *           _new<ATextField>() AUI_LET { it->setText("Hello!") },
 *           _new<ATextField>() AUI_LET { it->setText("World!") },
 *       });
 *       ```
 *     </td>
 *   </tr>
 * </table>
 */
#define AUI_LET ^ [&](const auto& it)

/**
 * @brief Allows to define a style to the view right in place.
 * @ingroup useful_macros
 * @details
 * ```cpp
 * #include <AUI/ASS/ASS.h>
 * using namespace ass;
 * ...
 * setContents(Centered {
 *   _new<ALabel>("Red text!") AUI_WITH_STYLE { TextColor { AColor::RED } },
 * });
 * ```
 *
 * Also applicable to declarative-style views:
 * ```cpp
 * #include <AUI/ASS/ASS.h>
 * using namespace ass;
 * ...
 * setContents(Centered {
 *   Label { "Red text!" } AUI_WITH_STYLE { TextColor { AColor::RED } },
 * });
 * ```
 */
#define AUI_WITH_STYLE & ass::PropertyListRecursive

/**
 * @brief Executes following {} block asynchronously in the [global](AThreadPool::global()) thread pool. Unlike
 * AUI_THREADPOOL_X, does now allow to set lambda's capture. Lambda's capture is `[=]`.
 *
 * @ingroup useful_macros
 * @return <code>AFuture<T></code> where <code>T</code> is the return type of the lambda.
 * @details
 * When <code>AFuture<T></code> is destroyed, the corresponding `AUI_THREADPOOL` task is either cancelled or removed from
 * the execution queue. Use AFutureSet or AAsyncHolder to keep multiple AFuture<T> alive.
 *
 * Example without a return value:
 * ```cpp
 * auto task = AUI_THREADPOOL {
 *   AThread::sleep(1000); // a long task
 * };
 * ```
 *
 * Example with a return value:
 *
 * ```cpp
 * auto futureStatus = AUI_THREADPOOL {
 *   int status;
 *   ...
 *   AThread::sleep(1000); // a long task
 *   ...
 *   return status;
 * };
 * int status = *futureStatus;
 * ```
 *
 * Lambda operators are supported:
 * ```cpp
 * auto futureStatus = AUI_THREADPOOL mutable noexcept {
 *   int status;
 *   ...
 *   AThread::sleep(1000); // a long task
 *   ...
 *   return status;
 * };
 * int status = *futureStatus;
 * ```
 */
#define AUI_THREADPOOL AThreadPool::global() * [=]()


/**
 * @brief Executes following {} block asynchronously in the [global](AThreadPool::global()) thread pool. Unlike
 * AUI_THREADPOOL, allows to set lambda's capture but you should always specify lambda's capture.
 *
 * @ingroup useful_macros
 * @return <code>AFuture<T></code> where <code>T</code> is the return type of the lambda.
 * @details
 * When <code>AFuture<T></code> is destroyed, the corresponding `AUI_THREADPOOL` task is either cancelled or removed from
 * the execution queue. Use AFutureSet or AAsyncHolder to keep multiple AFuture<T> alive.
 *
 * Example without a return value:
 *
 * ```cpp
 * auto task = AUI_THREADPOOL_X [&] {
 *   AThread::sleep(1000); // a long task
 * };
 * ```
 *
 * Example with a return value:
 * ```cpp
 * auto futureStatus = AUI_THREADPOOL_X [&] {
 *   int status;
 *   ...
 *   AThread::sleep(1000); // a long task
 *   ...
 *   return status;
 * };
 * int status = *futureStatus;
 * ```
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
 *       ```cpp
 *       static bool done = false;
 *       if (!done) {
 *           done = true;
 *           std::printf("Only once!");
 *       }
 *       ```
 *     </td>
 *     <td>
 *       ```cpp
 *       AUI_DO_ONCE {
 *           std::printf("Only once!");
 *       }
 *       ```
 *     </td>
 *   </tr>
 * </table>
 */
#define AUI_DO_ONCE if(static bool _aui_once = false; (!_aui_once && (_aui_once = true)))

/**
 * @brief Executes lambda on main thread.
 * @ingroup useful_macros
 */
#define AUI_UI_THREAD (*AThread::main()) * [=]()

/**
 * @brief Executes lambda on main thread. Allows to determine lambda's capture.
 * @ingroup useful_macros
 */
#define AUI_UI_THREAD_X (*AThread::main()) *

#define AUI_REPEAT(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex)
#define AUI_REPEAT_ASYNC(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex) AThreadPool::global() << [=]()