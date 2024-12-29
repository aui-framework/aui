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

namespace aui::signal {
template<typename Lambda>
concept NoArgInvocation = requires(Lambda&& lambda) {
    { lambda() };
};

template<typename Lambda, typename Projection, typename... Args>
concept DirectInvocation = requires(Lambda&& lambda, Projection&& projection, Args&&... args) {
    { lambda(std::invoke(projection, std::forward<Args>(args)...)) };
};

template<typename Lambda, typename Projection, typename... Args>
concept TupleToValueInvocation = requires(Lambda&& lambda, Projection&& projection, Args&&... args) {
    { lambda(std::invoke(projection, std::make_tuple(std::forward<Args>(args)...))) };
};

template<typename Lambda, typename Projection, typename... Args>
concept TupleToTupleInvocation = requires(Lambda&& lambda, Projection&& projection, Args&&... args) {
    { std::apply(lambda, std::invoke(projection, std::make_tuple(std::forward<Args>(args)...))) }; // *** READ THIS ***
    // if your code breaks above, it's because your call to AObject::connect is ill-formed. Please check that your
    // arguments are compatible. Define a projection if necessary.
};
}

template<typename... Args>
class ASignal final: public AAbstractSignal
{
    friend class AObject;

    template <typename T>
    friend class AWatchable;
public:
    using func_t = std::function<void(Args...)>;
    using args_t = std::tuple<Args...>;

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

    template <aui::convertible_to<AObjectBase*> Object, typename Invocable, typename Projection>
    void connect(Object objectBase, Invocable&& invocable, Projection&& projection) {
        AObject* object = nullptr;
        if constexpr (requires { object = objectBase; }) {
            object = objectBase;
        }
        mSlots.push_back(_new<slot>(slot {
          objectBase, object,
          makeCallable(objectBase, std::forward<Invocable>(invocable), std::forward<Projection>(projection)) }));
        linkSlot(objectBase);
    }

    template<typename Lambda, typename... A>
    struct argument_ignore_helper {};

    // empty arguments
    template<typename Lambda>
    struct argument_ignore_helper<void(Lambda::*)() const>
    {
        Lambda l;

        explicit argument_ignore_helper(Lambda l)
                : l(l)
        {
        }

        template<typename... Others>
        void operator()(Others&... args) {
            l();
        }
    };

    template<typename Lambda, typename A1>
    struct argument_ignore_helper<void(Lambda::*)(A1) const>
    {
        Lambda l;

        explicit argument_ignore_helper(Lambda l)
                : l(l)
        {
        }

        template<typename... Others>
        void operator()(A1&& a1, Others&...) {
            l(std::forward<A1>(a1));
        }
    };
    template<typename Lambda, typename A1, typename A2>
    struct argument_ignore_helper<void(Lambda::*)(A1, A2) const>
    {
        Lambda l;

        explicit argument_ignore_helper(Lambda l)
                : l(l)
        {
        }

        template<typename... Others>
        void operator()(A1&& a1, A2&& a2, Others&...)
        {
            l(std::forward<A1>(a1), std::forward<A2>(a2));
        }
    };

    template<typename Lambda, typename A1, typename A2, typename A3>
    struct argument_ignore_helper<void(Lambda::*)(A1, A2, A3) const>
    {
        Lambda l;

        explicit argument_ignore_helper(Lambda l)
                : l(l)
        {
        }

        template<typename... Others>
        void operator()(A1&& a1, A2&& a2, A3&& a3, Others...)
        {
            l(std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3));
        }
    };

    template <class Lambda, typename Projection>
    static auto makeCallableImpl(Lambda&& lambda, Projection&& projection)
        requires aui::signal::NoArgInvocation<Lambda> ||
                 aui::signal::DirectInvocation<Lambda, Projection, Args...> ||         // hope this gives proper compile
                 aui::signal::TupleToValueInvocation<Lambda, Projection, Args...> ||   // -time diagnostics
                 aui::signal::TupleToTupleInvocation<Lambda, Projection, Args...>      //
    {
        return [lambda = std::forward<Lambda>(lambda),
                projection = std::forward<Projection>(projection)](Args... args) mutable {
            if constexpr (aui::signal::NoArgInvocation<Lambda>) {
                AUI_MARK_AS_USED(projection);
                lambda();
            } else if constexpr (aui::signal::DirectInvocation<Lambda, Projection, Args...>) {
                lambda(std::invoke(projection, std::forward<Args>(args)...));
            } else if constexpr (aui::signal::TupleToTupleInvocation<Lambda, Projection, Args...>) {
                std::apply(lambda, std::invoke(projection, std::make_tuple(std::forward<Args>(args)...)));
            } else if constexpr (aui::signal::TupleToValueInvocation<Lambda, Projection, Args...>) {
                lambda(std::invoke(projection, std::make_tuple(std::forward<Args>(args)...)));
            }
        };
    }

    // Lambda function
    template<class Object, class Lambda, typename Projection>
    static auto makeCallable(Object object, Lambda&& lambda, Projection&& projection)
    {
        static_assert(std::is_class_v<std::decay_t<Lambda>>, "the lambda should be a class");
        return makeCallableImpl(argument_ignore_helper<decltype(&std::decay_t<Lambda>::operator())>(std::forward<Lambda>(lambda)), std::forward<Projection>(projection));
    }

    // Member function
    template<class Derived, class Object, typename... FArgs, typename Projection>
    static auto makeCallable(Derived derived, void(Object::* memberFunction)(FArgs...), Projection&& projection)
    {
        auto* object = static_cast<Object*>(derived);
        return makeCallable(object, [object, memberFunction](FArgs... args)
        {
          (object->*memberFunction)(args...);
        }, std::forward<Projection>(projection));
    }

public:

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
    bool hasConnectionsWith(aui::no_escape<AObjectBase> object) noexcept {
        return std::any_of(mSlots.begin(), mSlots.end(), [&](const _<slot>& s) {
            return s->objectBase == object.ptr();
        });
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

        if (!receiverPtr || receiverPtr->isSignalsEnabled()) {
            (std::apply)(slot.func, args);
            if (AAbstractSignal::isDisconnected()) {
                unlinkSlot(slot.object);
                i = slots.erase(i);
                continue;
            }
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
 * @ingroup core
 * @ingroup signal_slot
 * See @ref signal_slot "signal-slot system" for more info.
 */
template<typename... Args>
using emits = ASignal<Args...>;

#define signals public