/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
