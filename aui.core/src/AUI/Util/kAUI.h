/**
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
 * \brief Passes the current class and type of the current class separated by comma. It's convenient to use with the
 *        connect function:
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
 * \brief Passes some variable and type of the variable separated by comma. It's convenient to use with the connect
 *        function:
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
#define slot(v) v, &std::decay_t<decltype(v)>::stored_t

/**
 * \brief Performs multiple operations on a single object without repeating its name.
 * \param object object to perform operations on
 * \param lambda code executed in the context of an object (as its member function)
 * \note analogue to <code>with</code>, <code>apply</code> in Kotlin
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
            lambda;                                                            \
        }                                                                      \
    };                                                                         \
    (*reinterpret_cast<__apply ## __FUNCTION__ ## __LINE__ *>(object.get()))()

/**
 * \brief Performs multiple operations on a single object without repeating its name (in place)
 *        This function can be used as an operator on object.
 * \param T object type to perform operations on
 * \param lambda code executed in the context of an object (as its member function)
 * \note analogue to <code>with</code>, <code>apply</code> in Kotlin
 * \example
 * <code>
 * class Worker {<br />
 * public:<br />
 * &#09;void buildHouse();<br />
 * &#09;void plantTree();<br />
 * &#09;void raiseSon();<br />
 * };<br />
 * ...<br />
 * auto worker = _new&lt;Worker&gt;() let {<br />
 * &#09;it->buildHouse();<br />
 * &#09;it->plantTree();<br />
 * &#09;it->raiseSon();<br />
 * };<br />
 * </code>
 */
#define let ^ [&](const auto& it)

#define with_style + RuleWithoutSelector

/**
 * \brief Executes following {} block asynchronously in standard thread pool. Does now allow to set lambda's capture.
 * \return nothing, if lambda does not contain <code>return</code>; <code>_&lt;AFuture&lt;T&gt;&gt;</code> if lambda
 *         contains <code>return</code> where <code>T</code> - return type of the lambda.
 * \see asyncX
 * \example
 * <p>Example without return value</p>
 * <code>
 * async {<br />
 * &#09;...<br />
 * };<br />
 * </code>
 * <p>Example with return value</p>
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
 * \brief Executes following {} block asynchronously in standard thread pool. Allows to set lambda's capture but you
 *        should always specify lambda's capture.
 * \return nothing, if lambda does not contain <code>return</code>; <code>_&lt;AFuture&lt;T&gt;&gt;</code> if lambda
 *         contains <code>return</code> where <code>T</code> - return type of the lambda.
 * \see async
 * \example
 * <p>Example without return value</p>
 * <code>
 * asyncX [&] {<br />
 * &#09;...<br />
 * };<br />
 * </code>
 * <p>Example with return value</p>
 * <code>
 * auto futureStatus = asyncX [&] {<br />
 * &#09;int status;<br />
 * &#09;...<br />
 * &#09;return status;<br />
 * };<br />
 * int status = **futureStatus;<br />
 * </code>
 */
#define asyncX AThreadPool::global() *

/**
 * \brief Executes following function call or {} block once per program execution.
 * \example
 * <p>Example with {} block</p>
 * <code>
 * do_once {<br />
 * &#09;aui::importPlugin("MyPlugin");<br />
 * }<br />
 * </code>
 * <p>Example with function call</p>
 * <code>
 * do_once aui::importPlugin("MyPlugin");
 * </code>
 */
#define do_once static uint8_t _aui_once = 0; if(!_aui_once++)

/**
 * \brief Executes lambda on current object's thread.
 */
#define ui_thread (*getThread()) * [=]()

/**
 * \brief Executes lambda on current object's thread. Allows to determine lambda's capture.
 */
#define ui_threadX (*getThread()) *
#define repeat(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex)
#define repeat_async(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex) AThreadPool::global() << [=]()