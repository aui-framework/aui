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

namespace aui::impl {
    /**
     * This class calls cancel() and wait() methods of AFuture::Inner BEFORE AFuture::Inner destruction in order to keep
     * alive the weak reference created in AThreadPool::operator<<.
     */
    template<typename Inner>
    struct CancellationWrapper {
        _<Inner> wrapped;

        explicit CancellationWrapper(_<Inner> wrapped) : wrapped(std::move(wrapped)) {}

        ~CancellationWrapper() {
            wrapped->cancel();
            wrapped->wait();
        }
        Inner* operator->() const noexcept {
            return wrapped.get();
        }
    };
}


template<typename Value = void>
class AFuture
{
friend class AThreadPool;
private:
    struct Inner {
        bool interrupted = false;
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
            return value || exception || interrupted;
        }

        bool setThread(_<AAbstractThread> thr) noexcept {
            std::unique_lock lock(mutex);
            if (cancelled) return true;
            thread = std::move(thr);
            return false;
        }

        void wait() noexcept {
            std::unique_lock lock(mutex);
            while ((thread || !cancelled) && !hasResult()) {
                cv.wait(lock);
            }
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

        void reportInterrupted() noexcept {
            std::unique_lock lock(mutex);
            interrupted = true;
            cv.notify_all();
        }

        void reportException(const AException& e) noexcept {
            std::unique_lock lock(mutex);
            exception = AInvocationTargetException(e.getMessage(), AReflect::name(&e)); // NOLINT(bugprone-throw-keyword-missing)
            cv.notify_all();
            nullsafe(onError)(*exception);
        }

    };
    _<aui::impl::CancellationWrapper<Inner>> mInner;


public:
    AFuture() noexcept: mInner(_new<aui::impl::CancellationWrapper<Inner>>(aui::ptr::manage(new Inner))) {}


    void result(Value v) noexcept {
        (*mInner)->result(std::move(v));
    }

    void reportException(const AException& e) noexcept {
        (*mInner)->reportException(e);
    }

    template<typename Callback>
    AFuture& onSuccess(Callback&& callback) noexcept {
        std::unique_lock lock((*mInner)->mutex);
        (*mInner)->onSuccess = [innerStorage = mInner, callback = std::forward<Callback>(callback)](const Value& v) {
            callback(v);
        };
        return *this;
    }

    template<typename Callback>
    AFuture& onError(Callback&& callback) noexcept {
        std::unique_lock lock((*mInner)->mutex);
        (*mInner)->onError = [innerStorage = mInner, callback = std::forward<Callback>(callback)](const AException& v) {
            callback(v);
        };
        return *this;
    }


    void cancel() noexcept {
        (*mInner)->cancel();
    }

    void reportInterrupted() {
        (*mInner)->reportInterrupted();
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
        (*mInner)->wait();
        if ((*mInner)->exception) throw *(*mInner)->exception;
        return *(*mInner)->value;
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

template<>
class AFuture<void>
{
friend class AThreadPool;
private:
    struct Inner {
        bool value = false;
        bool interrupted = false;
        std::optional<AInvocationTargetException> exception;
        AMutex mutex;
        AConditionVariable cv;
        AAbstractThread* worker = nullptr;
        std::function<void()> onSuccess;
        std::function<void(const AException& exception)> onError;
        _<AAbstractThread> thread;
        bool cancelled = false;

        [[nodiscard]]
        bool hasResult() const noexcept {
            return value || exception || interrupted;
        }

        void reportInterrupted() noexcept {
            std::unique_lock lock(mutex);
            interrupted = true;
            cv.notify_all();
        }

        bool setThread(_<AAbstractThread> thr) noexcept {
            std::unique_lock lock(mutex);
            if (cancelled) return true;
            thread = std::move(thr);
            return false;
        }

        /**
         * Whats for result or cancellation or interruption or exception.
         */
        void wait() noexcept {
            std::unique_lock lock(mutex);
            while ((thread || !cancelled) && !hasResult()) {
                cv.wait(lock);
            }
        }

        [[nodiscard]]
        bool isWaitNeeded() noexcept {
            return (thread || !cancelled) && !hasResult();
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

        void result() noexcept {
            std::unique_lock lock(mutex);
            value = true;
            cv.notify_all();
            nullsafe(onSuccess)();
        }

        void reportException(const AException& e) noexcept {
            std::unique_lock lock(mutex);
            exception = AInvocationTargetException(e.getMessage(), AReflect::name(&e)); // NOLINT(bugprone-throw-keyword-missing)
            cv.notify_all();
            nullsafe(onError)(*exception);
        }

        ~Inner() {
            cancel();
            wait();
        }
    };
    _<aui::impl::CancellationWrapper<Inner>> mInner;


public:
    AFuture() noexcept: mInner(_new<aui::impl::CancellationWrapper<Inner>>(aui::ptr::manage(new Inner))) {}


    void result() noexcept {
        (*mInner)->result();
    }

    /**
     * @return true when the task finished no matter successfully or with exception
     */
    [[nodiscard]]
    bool hasResult() noexcept {
        return (*mInner)->hasResult();
    }


    /**
     * @return true when <a href="wait()">wait()</a> function will block the execution
     */
    [[nodiscard]]
    bool isWaitNeeded() noexcept {
        return (*mInner)->isWaitNeeded();
    }

    /**
     * Whats for result or cancellation or interruption or exception.
     */
    void wait() const noexcept {
        (*mInner)->wait();
    }

    void reportException(const AException& e) noexcept {
        (*mInner)->reportException(e);
    }

    template<typename Callback>
    AFuture& onSuccess(Callback&& callback) noexcept {
        std::unique_lock lock((*mInner)->mutex);
        (*mInner)->onSuccess = [innerStorage = mInner, callback = std::forward<Callback>(callback)]() {
            callback();
        };
        return *this;
    }

    template<typename Callback>
    AFuture& onError(Callback&& callback) noexcept {
        std::unique_lock lock((*mInner)->mutex);
        (*mInner)->onError = [innerStorage = mInner, callback = std::forward<Callback>(callback)]() {
            callback();
        };
        return *this;
    }

    void cancel() noexcept {
        (*mInner)->cancel();
    }
    void reportInterrupted() noexcept {
        (*mInner)->reportInterrupted();
    }

    /**
     * Sleeps if the result is not currently available.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
     * </dl>
     * @return the object stored from the another thread.
     */
    void operator*() const {
        wait();
        if ((*mInner)->exception) throw *(*mInner)->exception;
    }
};

#include <AUI/Thread/AThreadPool.h>
#include <AUI/Common/AException.h>
