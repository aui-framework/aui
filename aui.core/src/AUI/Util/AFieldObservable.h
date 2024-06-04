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
#include <utility>
#include "AUI/Common/ASignal.h"
#include "AUI/Traits/concepts.h"
#include "AUI/Util/ADataBinding.h"

template<typename T, aui::invocable<const T&> AdapterCallable>
struct AFieldObservableAdapter;

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
        return mValue;
    }
    T& operator-=(T t) {
        mValue -= t;
        notifyObservers();
        return mValue;
    }
    T& operator*=(T t) {
        mValue *= t;
        notifyObservers();
        return mValue;
    }
    T& operator/=(T t) {
        mValue /= t;
        notifyObservers();
        return mValue;
    }

    operator const T&() const noexcept
    {
        return mValue;
    }

    [[nodiscard]]
    const T& value() const noexcept {
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

    /**
     * @brief Removes an observer.
     */
    void operator>>(ObserverHandle h) {
        removeObserver(h);
    }

    /**
     * @brief Removes an observer.
     */
    void removeObserver(ObserverHandle h) {
        mObservers.erase(std::remove_if(mObservers.begin(), mObservers.end(), [&](const Observer& o) {
            return &o == h;
        }), mObservers.end());
    }

    template<aui::invocable<const T&> AdapterCallable>
    AFieldObservableAdapter<T, std::decay_t<AdapterCallable>> operator()(AdapterCallable&& callable) noexcept;

private:
    T mValue;
    std::list<Observer> mObservers;
};

template<typename T, aui::invocable<const T&> AdapterCallable>
struct AFieldObservableAdapter {
    AFieldObservable<T>& field;
    AdapterCallable callable;

    using return_t = decltype(callable(std::declval<T>()));
};

template<typename T>
template<aui::invocable<const T&> AdapterCallable>
AFieldObservableAdapter<T, std::decay_t<AdapterCallable>> AFieldObservable<T>::operator()(AdapterCallable&& callable) noexcept {
    return { .field = *this, .callable = std::forward<AdapterCallable>(callable) };
}


template<typename View, typename Data>
_<View> operator&&(const _<View>& object, AFieldObservable<Data>& observable) {
    using ObserverHandle = typename std::decay_t<decltype(observable)>::ObserverHandle;
    auto observerHandle = _new<ObserverHandle>(nullptr);
    if (ADataBindingDefault<View, Data>::getSetter()) {
        *observerHandle = observable << [weak = object.weak(), observerHandle, observable = &observable](const Data& newValue) {
            auto object = weak.lock();
            if (object == nullptr) {
                observable->removeObserver(*observerHandle);
            }
            (object.get()->*ADataBindingDefault<View, Data>::getSetter())(newValue);
        };
    }
    if (auto getter = ADataBindingDefault<View, Data>::getGetter()) {
        AObject::connect(object.get()->*getter, object, [&observable, observerHandle = *observerHandle](Data newValue) {
            observable.setValue(std::move(newValue), observerHandle);
        });
    }

    return object;
}

template<typename View, typename ModelData, typename AdapterCallback>
_<View> operator&&(const _<View>& object, AFieldObservableAdapter<ModelData, AdapterCallback> observableAdapter) {
    using Data = AFieldObservableAdapter<ModelData, AdapterCallback>::return_t;
    auto& observable = observableAdapter.field;
    using ObserverHandle = typename std::decay_t<decltype(observable)>::ObserverHandle;
    auto observerHandle = _new<ObserverHandle>(nullptr);
    if (ADataBindingDefault<View, Data>::getSetter()) {
        *observerHandle = observable << [weak = object.weak(), observable = &observable, observerHandle, transformer = std::move(observableAdapter.callable)](const ModelData& newValue) {
            auto object = weak.lock();
            if (object == nullptr) {
                observable->removeObserver(*observerHandle);
                return;
            }
            (object.get()->*ADataBindingDefault<View, Data>::getSetter())(transformer(newValue));
        };
    }

    return object;
}

