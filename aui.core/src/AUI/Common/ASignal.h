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

#include <algorithm>
#include <functional>
#include "AUI/Common/ADeque.h"
#include "AUI/Common/AObject.h"
#include "AUI/Thread/AMutex.h"
#include "AAbstractSignal.h"
#include "AUI/Traits/values.h"

namespace aui::detail::signal {
template<size_t I, typename TupleInitial, typename TupleAll>
auto resizeTuple(TupleInitial initial, TupleAll all) {
    if constexpr (I == 0) {
        return initial;
    } else {
        return resizeTuple<I - 1>(std::tuple_cat(initial, std::make_tuple(std::get<std::tuple_size_v<TupleInitial>>(all))), all);
    }
}

template<aui::not_overloaded_lambda Lambda, typename... Args>
inline void callIgnoringExcessArgs(Lambda&& lambda, const Args&... args) {
    static constexpr size_t EXPECTED_ARG_COUNT = std::tuple_size_v<typename lambda_info<std::decay_t<Lambda>>::args>;
    auto smallerTuple = resizeTuple<EXPECTED_ARG_COUNT>(std::make_tuple(), std::make_tuple(std::cref(args)...));
    std::apply(lambda, smallerTuple);
}

template <typename Projection>
struct projection_info {
    static_assert(
        aui::pointer_to_member<Projection> || aui::not_overloaded_lambda<Projection> ||
            aui::function_pointer<Projection>,
        "projection is required to be an pointer-to-member or not overloaded lambda or function pointer");
};

template<aui::pointer_to_member Projection>
struct projection_info<Projection> {
private:
    template <typename... T>
    struct cat;

    template <typename First, typename... T>
    struct cat<First, std::tuple<T...>> {
        using type = std::tuple<First, T...>;
    };

public:
    using info = typename aui::member<Projection>;
    using return_t = typename info::return_t;
    using args = typename cat<typename info::clazz&, typename info::args>::type;
};

template<aui::not_overloaded_lambda Projection>
struct projection_info<Projection> {
    using info = typename  aui::lambda_info<Projection>;
    using return_t = typename info::return_t;
    using args = typename info::args;
};

template<aui::function_pointer Projection>
struct projection_info<Projection> {
    using info = typename aui::function_info<Projection>;
    using return_t = typename info::return_t;
    using args = typename info::args;
};


template<typename AnySignal, // can't use AAnySignal here, as concept would depend on itself
         typename Projection>
struct ProjectedSignal {
    friend class ::AObject;

    AnySignal& base;
    std::decay_t<Projection> projection;

    ProjectedSignal(AnySignal& base, Projection projection) : base(base), projection(std::move(projection)) {}

    using projection_info_t = aui::detail::signal::projection_info<Projection>;

    using projection_returns_t = typename projection_info_t::return_t;

    static constexpr bool IS_PROJECTION_RETURNS_TUPLE = aui::is_tuple<projection_returns_t>;

    using emits_args_t =  std::conditional_t<IS_PROJECTION_RETURNS_TUPLE,
                                       projection_returns_t,
                                       std::tuple<projection_returns_t>>;

    template <convertible_to<AObjectBase*> Object, not_overloaded_lambda Lambda>
    void connect(Object objectBase, Lambda&& lambda) {
        base.connect(
            objectBase,
            tuple_visitor<typename projection_info_t::args>::for_each_all([&]<typename... ProjectionArgs>() {
                return [invocable = std::forward<Lambda>(lambda),
                        projection = projection](const std::decay_t<ProjectionArgs>&... args) {
                    auto result = std::invoke(projection, args...);
                    if constexpr (IS_PROJECTION_RETURNS_TUPLE) {
                        std::apply(invocable, std::move(result));
                    } else {
                        std::invoke(invocable, std::move(result));
                    }
                };
            }));
    }

    operator bool() const {
        return bool(base);
    }

private:
    template <not_overloaded_lambda Lambda>
    auto makeRawInvocable(Lambda&& lambda) const {
        return tuple_visitor<emits_args_t>::for_each_all([&]<typename... ProjectionResult>() {
            return [lambda = std::forward<Lambda>(lambda)](const std::decay_t<ProjectionResult>&... args){
                aui::detail::signal::callIgnoringExcessArgs(lambda, args...);
            };
        });
    }
};

}

template<typename... Args>
class ASignal final: public AAbstractSignal
{
    friend class AObject;
    friend class UIDataBindingTest_APropertyPrecomputed_Complex_Test;
    template<typename AnySignal,
              typename Projection>
    friend struct aui::detail::signal::ProjectedSignal;

    template <typename T>
    friend class AWatchable;
public:
    using func_t = std::function<void(Args...)>;
    using emits_args_t = std::tuple<Args...>;

    template<typename Projection>
    auto projected(Projection&& projection) const {
        return aui::detail::signal::ProjectedSignal(const_cast<ASignal&>(*this), std::forward<Projection>(projection));
    }

    struct call_wrapper {
        ASignal& signal;
        std::tuple<Args...> args;

        void invokeSignal(AObject* emitter) {
            signal.invokeSignal(emitter, std::move(args));
        }
    };

    call_wrapper operator()(Args... args) {
        return {*this, std::make_tuple(std::move(args)...)};
    }

    ASignal() = default;
    ASignal(ASignal&&) noexcept = default;
    ASignal(const ASignal&) = delete;

    virtual ~ASignal() noexcept
    {
        for (const _<slot>& slot : mSlots)
        {
            unlinkSlot(slot->objectBase);
        }
    }

    /**
     * Check whether signal contains any connected slots or not. It's very useful then signal argument values
     * calculation is expensive and you do not want to calculate them if no signals connected to the slot.
     * @return true, if slot contains any connected slots, false otherwise.
     */
    operator bool() const {
        return !mSlots.empty();
    }

    void clearAllConnections() const noexcept override
    {
        clearAllConnectionsIf([](const auto&){ return true; });
    }
    void clearAllConnectionsWith(aui::no_escape<AObjectBase> object) const noexcept override
    {
        clearAllConnectionsIf([&](const _<slot>& p){ return p->objectBase == object.ptr(); });
    }

    [[nodiscard]]
    bool hasConnectionsWith(aui::no_escape<AObjectBase> object) const noexcept override {
        return std::any_of(mSlots.begin(), mSlots.end(), [&](const _<slot>& s) {
            return s->objectBase == object.ptr();
        });
    }

private:
    struct slot
    {
        AObjectBase* objectBase;
        AObject* object;
        func_t func;
        bool isDisconnected = false;
    };

    mutable AVector<_<slot>> mSlots;

    void invokeSignal(AObject* emitter, const std::tuple<Args...>& args = {});

    template <aui::convertible_to<AObjectBase*> Object, aui::not_overloaded_lambda Lambda>
    void connect(Object objectBase, Lambda&& lambda) {
        AObject* object = nullptr;
        if constexpr (requires { object = objectBase; }) {
            object = objectBase;
        }
        mSlots.push_back(_new<slot>(slot { objectBase, object, makeRawInvocable(std::forward<Lambda>(lambda)) }));
        linkSlot(objectBase);
    }

    void addGenericObserver(AObjectBase* object, std::function<void()> observer) override {
        connect(object, [observer = std::move(observer)] {
            observer();
        });
    }

    template<aui::not_overloaded_lambda Lambda>
    auto makeRawInvocable(Lambda&& lambda) const
    {
        return [lambda = std::forward<Lambda>(lambda)](const Args&... args){
            aui::detail::signal::callIgnoringExcessArgs(lambda, args...);
        };
    }

private:

    template<typename Predicate>
    void clearAllConnectionsIf(Predicate&& predicate) const noexcept {
        /*
         * Removal of connections before end of execution of clearAllConnectionsIf may cause this ASignal destruction,
         * causing undefined behaviour. Destructing these connections after mSlotsLock unlocking solves the problem.
         */
        AVector<func_t> slotsToRemove;

        slotsToRemove.reserve(mSlots.size());
        mSlots.removeIf([&slotsToRemove, predicate = std::move(predicate)](const _<slot>& p) {
            if (predicate(p)) {
                slotsToRemove << std::move(p->func);
                return true;
            }
            return false;
        });

        slotsToRemove.clear();
    }
};
#include <AUI/Thread/AThread.h>

template <typename ... Args>
void ASignal<Args...>::invokeSignal(AObject* emitter, const std::tuple<Args...>& args)
{
    if (mSlots.empty())
        return;

    _<AObject> emitterPtr, receiverPtr;

    if (emitter != nullptr) {
        if (auto sharedPtr = weakPtrFromObject(emitter).lock()) { // avoid emitter removal during signal processing
            emitterPtr = std::move(static_cast<_<AObject>>(sharedPtr));
        }
    }

    auto slots = std::move(mSlots); // needed to safely iterate through the slots
    for (auto i = slots.begin(); i != slots.end();)
    {
        slot& slot = **i;
        _weak<AObject> receiverWeakPtr;
        if (slot.object != nullptr) {
            receiverWeakPtr = weakPtrFromObject(slot.object);
            if (slot.object->isSlotsCallsOnlyOnMyThread() && slot.object->getThread() != AThread::current()) {
                // perform crossthread call; should make weak ptr to the object and queue call to thread message queue

                /*
                 * That's because shared_ptr counting mechanism is used when doing a crossthread call.
                 * It could not track the object existence without shared_ptr block.
                 * Also, receiverWeakPtr.lock() may be null here because object is in different thread and being destructed by shared_ptr but have not reached clearSignals() yet.
                 */
                if (receiverWeakPtr.lock() != nullptr) {
                    slot.object->getThread()->enqueue(
                        [this, receiverWeakPtr = std::move(receiverWeakPtr), slot = *i, args = args]() {
                            if (slot->isDisconnected) {
                                return;
                            }
                            if (auto receiverPtr = receiverWeakPtr.lock()) {
                                AAbstractSignal::isDisconnected() = false;
                                (std::apply)(slot->func, args);
                                if (AAbstractSignal::isDisconnected()) {
                                    unlinkSlot(receiverPtr.get());
                                    slot->isDisconnected = true;
                                    mSlots.removeFirst(slot);
                                }
                            }
                        });
                }
                ++i;
                continue;
            }
        }
        AAbstractSignal::isDisconnected() = false;

        if (auto sharedPtr = receiverWeakPtr.lock()) { // avoid receiver removal during signal processing
            receiverPtr = std::move(sharedPtr);
        }

        (std::apply)(slot.func, args);
        if (AAbstractSignal::isDisconnected()) {
            unlinkSlot(slot.object);
            i = slots.erase(i);
            continue;
        }
        ++i;
    }
    AUI_MARK_AS_USED(emitterPtr);
    AUI_MARK_AS_USED(receiverPtr);

    if (mSlots.empty()) {
        mSlots = std::move(slots);
    } else {
        // mSlots might be modified by a single threaded signal call. In this case merge two vectors
        mSlots.insert(mSlots.begin(), std::make_move_iterator(slots.begin()), std::make_move_iterator(slots.end()));
    }

    AAbstractSignal::isDisconnected() = false;
}

/**
 * @brief A signal declaration.
 * @tparam Args signal arguments
 * @ingroup signal_slot
 * See @ref signal_slot "signal-slot system" for more info.
 */
template<typename... Args>
using emits = ASignal<Args...>;

#define signals public

/*

// UNCOMMENT THIS to test ProjectedSignal

static_assert(requires (aui::detail::signal::ProjectedSignal<emits<int>, decltype([](int) { return double(0);})> t) {
    requires !decltype(t)::IS_PROJECTION_RETURNS_TUPLE;
    { decltype(t)::base } -> aui::same_as<ASignal<int>&>;
    { decltype(t)::projection } -> aui::not_overloaded_lambda;
    { decltype(t)::projection_returns_t{} } -> aui::same_as<double>;
    { decltype(t)::emits_args_t{} } -> aui::same_as<std::tuple<double>>;
    { decltype(t)::projection_info_t::args{} } -> aui::same_as<std::tuple<int>>;
    { decltype(t)::projection_info_t::return_t{} } -> aui::same_as<double>;
});

static_assert(requires (aui::detail::signal::ProjectedSignal<emits<AString>, decltype(&AString::length)> t) {
    requires !decltype(t)::IS_PROJECTION_RETURNS_TUPLE;
    { decltype(t)::base } -> aui::same_as<ASignal<AString>&>;
    { decltype(t)::emits_args_t{} } -> aui::same_as<std::tuple<size_t>>;
    { decltype(t)::projection_returns_t{} } -> aui::same_as<size_t>;
});
*/
