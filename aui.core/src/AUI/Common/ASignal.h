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

#include <functional>
#include "AUI/Common/ADeque.h"
#include "AUI/Thread/AMutex.h"
#include "AAbstractSignal.h"


/**
 * @brief Represents a signal.
 * @tparam Args signal arguments
 * @ingroup core
 * @ingroup signal_slot
 */
template<typename... Args>
class ASignal final: public AAbstractSignal
{
    friend class AObject;

    template <typename T>
    friend class AWatchable;
public:
    using func_t = std::function<void(Args...)>;

private:
    struct slot
    {
        AObject* object; // TODO replace with weak_ptr
        func_t func;

        [[nodiscard]]
        bool operator==(const slot& rhs) const noexcept {
            return object == rhs.object && func.target_type() == rhs.func.target_type();
        }
    };

    std::recursive_mutex mSlotsLock;
    ADeque<slot> mSlots;

    void invokeSignal(AObject* emitter, const std::tuple<Args...>& args = {});

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

        void operator()(Args... args) {
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
        void call(A1&& a1, Others...)
        {
            l(std::forward<A1>(a1));
        }

        void operator()(Args&&... args) {
            call(std::forward<Args>(args)...);
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
        void call(A1&& a1, A2&& a2, Others...)
        {
            l(std::forward<A1>(a1), std::forward<A2>(a2));
        }

        void operator()(Args&&... args) {
            call(std::forward<Args>(args)...);
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
        void call(A1&& a1, A2&& a2, A3&& a3, Others...)
        {
            l(std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3));
        }

        void operator()(Args&&... args) {
            call(std::forward<Args>(args)...);
        }
    };



    // Member function
    template<class Derived, class Object, typename... FArgs>
    void connect(Derived derived, void(Object::* memberFunction)(FArgs...))
    {
        Object* object = static_cast<Object*>(derived);
        connect(object, [object, memberFunction](FArgs... args)
        {
            (object->*memberFunction)(args...);
        });
    }

    // Lambda function
    template<class Object, class Lambda>
    void connect(Object object, Lambda lambda)
    {
        static_assert(std::is_class_v<Lambda>, "the lambda should be a class");

        std::unique_lock lock(mSlotsLock);
        mSlots.push_back({ object, argument_ignore_helper<decltype(&Lambda::operator())>(lambda) });

        linkSlot(object);
    }

public:

    struct call_wrapper {
        ASignal& signal;
        std::tuple<Args...> args;

        void invokeSignal(AObject* emitter) {
            signal.invokeSignal(emitter, args);
        }
    };

    call_wrapper operator()(Args... args) {
        return {*this, std::make_tuple(std::move(args)...)};
    }

    ASignal() = default;
    ASignal(ASignal&&) = default;

    virtual ~ASignal() noexcept
    {
        std::unique_lock lock(mSlotsLock);
        for (slot& slot : mSlots)
        {
            unlinkSlot(slot.object);
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

    void clearAllConnections() noexcept override
    {
        clearAllConnectionsIf([](const auto&){ return true; });
    }
    void clearAllConnectionsWith(AObject* object) noexcept override
    {
        clearAllConnectionsIf([&](const slot& p){ return p.object == object; });
    }

private:

    template<typename Predicate>
    void clearAllConnectionsIf(Predicate&& predicate) noexcept {
        /*
         * Removal of connections before end of execution of clearAllConnectionsIf may cause this ASignal destruction,
         * causing undefined behaviour. Destructing these connections after mSlotsLock unlocking solves the problem.
         */
        AVector<func_t> slotsToRemove;

        std::unique_lock lock(mSlotsLock);
        slotsToRemove.reserve(mSlots.size());
        mSlots.removeIf([&slotsToRemove, predicate = std::move(predicate)](const slot& p) {
            if (predicate(p)) {
                slotsToRemove << std::move(p.func);
                return true;
            }
            return false;
        });
        lock.unlock();

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

    if (auto sharedPtr = weakPtrFromObject(emitter).lock()) { // avoid emitter removal during signal processing
        emitterPtr = std::move(static_cast<_<AObject>>(sharedPtr));
    }

    std::unique_lock lock(mSlotsLock);
    auto slots = std::move(mSlots); // needed to safely unlock the mutex
    for (auto i = slots.begin(); i != slots.end();)
    {
        auto receiverWeakPtr = weakPtrFromObject(i->object);
        if (i->object->isSlotsCallsOnlyOnMyThread() && i->object->getThread() != AThread::current())
        {
            // perform crossthread call; should make weak ptr to the object and queue call to thread message queue

            /*
             * That's because shared_ptr counting mechanism is used when doing a crossthread call.
             * It could not track the object existence without shared_ptr block.
             * Also, receiverWeakPtr.lock() may be null here because object is in different thread and being destructed
             * by shared_ptr but have not reached clearSignals() yet.
             */
            if (receiverWeakPtr.lock() != nullptr) {
                i->object->getThread()->enqueue([this,
                                                 receiverWeakPtr = std::move(receiverWeakPtr),
                                                 func = i->func,
                                                 args = args]() {
                    if (auto receiverPtr = receiverWeakPtr.lock()) {
                        AAbstractSignal::isDisconnected() = false;
                        (std::apply)(func, args);
                        if (AAbstractSignal::isDisconnected()) {
                            std::unique_lock lock(mSlotsLock);
                            unlinkSlot(receiverPtr.get());
                            slot s = { receiverPtr.get(), func };
                            mSlots.removeFirst(s);
                        }
                    }
                });
            }
            ++i;
        }
        else
        {
            AAbstractSignal::isDisconnected() = false;

            if (auto sharedPtr = receiverWeakPtr.lock()) { // avoid receiver removal during signal processing
                receiverPtr = std::move(sharedPtr);
            }

            (std::apply)(i->func, args);
            if (AAbstractSignal::isDisconnected()) {
                unlinkSlot(i->object);
                i = slots.erase(i);
            } else {
                ++i;
            }
        }
    }
    AUI_MARK_AS_USED(emitterPtr);
    AUI_MARK_AS_USED(receiverPtr);

    if (mSlots.empty()) {
        mSlots = std::move(slots);
    } else {
        mSlots.insert(mSlots.begin(), std::make_move_iterator(slots.begin()), std::make_move_iterator(slots.end()));
    }

    AAbstractSignal::isDisconnected() = false;
}

template<typename... Args>
using emits = ASignal<Args...>;

#define signals public