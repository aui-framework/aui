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

#include <atomic>
#include <functional>
#include <optional>
#include "AConditionVariable.h"
#include "AMutex.h"
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AException.h>

class AThreadPool;


class AInvocationTargetException: public AException {
private:
    AString mExceptionType;
    AString mMessage;

public:
    AInvocationTargetException(AString message, AString exceptionType):
        mMessage(std::move(message)),
        mExceptionType(std::move(exceptionType)) {}

    AString getMessage() const noexcept override {
        return "Exception of type " + mExceptionType + " has caught by AFuture: " + mMessage;
    }

    ~AInvocationTargetException() noexcept override = default;
};


template<typename Value>
class AFuture
{
private:
    Value* mValue = new Value;
    AMutex mMutex;
    AConditionVariable mNotify;
    std::optional<AInvocationTargetException> mException;
    std::function<void(const Value&)> mOnDone;
    std::atomic_int mRefCount = 2;

    void decRef()
    {
        if (--mRefCount == 0)
        {
            delete mValue;
        }
    }

    void notify() {
        mNotify.notify_one();
        if (mOnDone) {
            mOnDone(*mValue);
        }
    }


    AFuture() {}

public:

    template<typename Callable>
    static _<AFuture> make(AThreadPool& tp, Callable&& func);

    /**
     * Returns the result from the another thread. Sleeps if the result is not currently available.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
     * </dl>
     * @return the object stored from the another thread.
     */
    Value& operator*() {
        if (mRefCount == 2) {
            std::unique_lock lock(mMutex);
            while (mRefCount == 2)
            {
                mNotify.wait(lock);
            }
        }
        if (mException) throw *mException;
        return *mValue;
    }

    template<typename Object, typename Member>
    void onDone(const _<Object>& object, Member memberFunc) {
        std::unique_lock lock(mMutex);
        mOnDone = [object, memberFunc](const Value& t) {
            (object.get()->*memberFunc)(t);
        };
    }

    template<typename Object, typename Member>
    void onDone(const Object* object, Member memberFunc) {
        std::unique_lock lock(mMutex);
        mOnDone = [object, memberFunc](const Value& t) {
            (object->*memberFunc)(t);
        };
    }
    template<typename Callback>
    void onDone(const Callback& callable) {
        std::unique_lock lock(mMutex);
        mOnDone = [callable](const Value& t) {
            callable(t);
        };
    }

    ~AFuture()
    {
        decRef();
    }
};

#include <AUI/Thread/AThreadPool.h>
#include <AUI/Common/AException.h>
#include <AUI/Reflect/AReflect.h>

template <typename Value>
template <typename Callable>
_<AFuture<Value>> AFuture<Value>::make(AThreadPool& tp, Callable&& func)
{
    auto future = aui::ptr::manage(new AFuture<Value>);
    tp.run([future, func = std::forward<Callable>(func)]()
    {
        std::unique_lock lock(future->mMutex);
        try {
            *future->mValue = func();
        } catch (const AException& e) {
            future->mException = AInvocationTargetException(e.getMessage(), AReflect::name(&e));
        }
        future->decRef();
        future->notify();
    }, AThreadPool::PRIORITY_LOWEST);
    return future;
}
