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
#include <AUI/Reflect/AReflect.h>

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
    struct Inner {
        std::optional<Value> value;
        std::optional<AInvocationTargetException> exception;
        AMutex mutex;
        AConditionVariable cv;
        AAbstractThread* worker = nullptr;
        std::function<void(const Value& value)> onSuccess;
        std::function<void(const AException& exception)> onError;
        _<AAbstractThread> thread;
        bool cancelled = false;

        [[nodiscard]]
        bool hasResult() const noexcept {
            return value || exception;
        }

        bool setThread(_<AAbstractThread> thr) noexcept {
            std::unique_lock lock(mutex);
            if (cancelled) return true;
            thread = std::move(thr);
            return false;
        }

        void cancel() noexcept {
            std::unique_lock lock(mutex);
            if (!cancelled) {
                cancelled = true;
                if (thread && !hasResult()) {
                    thread->interrupt();
                }
            }
        }

        void result(Value v) noexcept {
            std::unique_lock lock(mutex);
            value = std::move(v);
            cv.notify_all();
            nullsafe(onSuccess)(*value);
        }

        void reportException(const AException& e) noexcept {
            std::unique_lock lock(mutex);
            exception = AInvocationTargetException(e.getMessage(), AReflect::name(&e)); // NOLINT(bugprone-throw-keyword-missing)
            cv.notify_all();
            nullsafe(onError)(*exception);
        }

        ~Inner() {
            cancel();
        }
    };
    _<Inner> mInner;


public:

    template<typename Callable>
    static AFuture make(AThreadPool& tp, Callable&& func) noexcept;

    AFuture() noexcept: mInner(aui::ptr::manage(new Inner)) {}


    void result(Value v) noexcept {
        mInner->result(std::move(v));
    }

    void reportException(const AException& e) noexcept {
        mInner->reportException(e);
    }

    template<typename Callback>
    AFuture& onSuccess(Callback&& callback) noexcept {
        std::unique_lock lock(mInner->mutex);
        mInner->onSuccess = [innerStorage = mInner, callback = std::forward<Callback>(callback)](const Value& v) {
            callback(v);
        };
        return *this;
    }

    template<typename Callback>
    AFuture& onError(Callback&& callback) noexcept {
        std::unique_lock lock(mInner->mutex);
        mInner->onError = [innerStorage = mInner, callback = std::forward<Callback>(callback)](const AException& v) {
            callback(v);
        };
        return *this;
    }

    void cancel() noexcept {
        mInner->cancel();
    }

    /**
     * Returns the result from the another thread. Sleeps if the result is not currently available.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
     * </dl>
     * @return the object stored from the another thread.
     */
    Value& operator*() {
        std::unique_lock lock(mInner->mutex);
        while (!(mInner->value || mInner->exception)) {
            mInner->cv.wait(lock);
        }
        if (mInner->exception) throw *mInner->exception;
        return *mInner->value;
    }

    /**
     * Returns the result from the another thread. Sleeps if the result is not currently available.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
     * </dl>
     * @return the object stored from the another thread.
     */
    Value* operator->() {
        return &operator*();
    }

    /**
     * Returns the result from the another thread. Sleeps if the result is not currently available.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
     * </dl>
     * @return the object stored from the another thread.
     */
    Value const * operator->() const {
        return &operator*();
    }

    /**
     * Returns the result from the another thread. Sleeps if the result is not currently available.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
     * </dl>
     * @return the object stored from the another thread.
     */
    const Value& operator*() const {
        return **const_cast<AFuture*>(this);
    }

};

#include <AUI/Thread/AThreadPool.h>
#include <AUI/Common/AException.h>
