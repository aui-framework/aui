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
#include "AUI/Common/ASignal.h"

/**
 * @brief Stores a value and observes it's changes, notifying observers.
 * @ingroup core
 */
template <typename T>
class AFieldObservable
{
public:
    using Observer = std::function<void()>;
    using ObserverHandle = Observer*;

    AFieldObservable(T initial = T()):
        mValue(std::move(initial))
    {
    }

    void setValue(T t, ObserverHandle exceptObserver = nullptr) {
        if (mValue != t) {
            mValue = std::move(t);
            notifyObservers(exceptObserver);
        }
    }

    void notifyObservers(ObserverHandle exceptObserver = nullptr) {
        for (const auto& v : mObservers) {
            if (&v != exceptObserver) {
                v();
            }
        }
    }

    AFieldObservable& operator=(T t)
    {
        setValue(t);
        return *this;
    }

    T& operator+=(T t) {
        mValue += t;
        notifyObservers();
        return t;
    }
    T& operator-=(T t) {
        mValue -= t;
        notifyObservers();
        return t;
    }
    T& operator*=(T t) {
        mValue *= t;
        notifyObservers();
        return t;
    }
    T& operator/=(T t) {
        mValue /= t;
        notifyObservers();
        return t;
    }

    operator const T&() const noexcept
    {
        return mValue;
    }

    T* operator->() noexcept {
        return &mValue;
    }

    T const * operator->() const noexcept {
        return &mValue;
    }

    /**
     * @brief Adds an observer, immediately feeding the observer with the current value.
     * @param observer observer. Optionally can accept the AFieldObservable's stored type as the first argument.
     */
    template<typename Observer_t>
    ObserverHandle addObserver(Observer_t&& observer) {
        constexpr bool expectsValue = !std::is_invocable_v<Observer_t>;
        if constexpr (expectsValue) {
            observer(mValue);
            mObservers.push_back([this, observer = std::forward<Observer_t>(observer)] {
                observer(mValue);
            });
        } else {
            observer();
            mObservers.push_back(std::forward<Observer_t>(observer));
        }

        return &mObservers.back();
    }

    /**
     * @brief Adds an observer, immediately feeding the observer with the current value.
     * @param observer observer. Optionally can accept the AFieldObservable's stored type as the first argument.
     */
    template<typename Observer_t>
    ObserverHandle operator<<(Observer_t observer) {
        return addObserver(std::forward<Observer_t>(observer));
    }

private:
    T mValue;
    std::list<Observer> mObservers;
};
