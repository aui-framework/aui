// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <list>
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
