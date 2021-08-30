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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <functional>
#include "AUI/Common/ADeque.h"
#include "AUI/Thread/AMutex.h"
#include "AAbstractSignal.h"

template<typename... Args>
class ASignal: public AAbstractSignal
{
    friend class AObject;

    template <typename T>
    friend class Watchable;
public:
    using func_t = std::function<void(Args...)>;

private:
    std::tuple<Args...> mArgs;

    struct slot
    {
        AObject* object; // TODO replace with weak_ptr
        func_t func;
    };

    AMutex mSlotsLock;
    ADeque<slot> mSlots;

    void invokeSignal();

    template<typename Lambda, typename... A>
    struct call_helper {};

    // empty arguments
    template<typename Lambda>
    struct call_helper<void(Lambda::*)() const>
    {
        Lambda l;

        explicit call_helper(Lambda l)
                : l(l)
        {
        }

        void operator()(Args... args) {
            l();
        }
    };

    template<typename Lambda, typename A1>
    struct call_helper<void(Lambda::*)(A1) const>
    {
        Lambda l;

        explicit call_helper(Lambda l)
                : l(l)
        {
        }

        template<typename... Others>
        void call(A1 a1, Others...)
        {
            l(a1);
        }

        void operator()(Args... args) {
            call(args...);
        }
    };
    template<typename Lambda, typename A1, typename A2>
    struct call_helper<void(Lambda::*)(A1, A2) const>
    {
        Lambda l;

        explicit call_helper(Lambda l)
                : l(l)
        {
        }

        template<typename... Others>
        void call(A1 a1, A2 a2, Others...)
        {
            l(a1, a2);
        }

        void operator()(Args... args) {
            call(args...);
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
        mSlots.push_back({ object, call_helper<decltype(&Lambda::operator())>(lambda) });

        linkSlot(object);
    }

public:
    ASignal<Args...>& operator()(const Args&... args) {
        mArgs = std::make_tuple(args...);
        return *this;
    }

    virtual ~ASignal()
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

    void clearAllConnections() override
    {
        std::unique_lock lock(mSlotsLock);
        mSlots.clear();
    }
    void clearAllConnectionsWith(AObject* object) override
    {
        std::unique_lock lock(mSlotsLock);
        for (auto it = mSlots.begin(); it != mSlots.end();)
        {
            if (it->object == object)
            {
                it = mSlots.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
};
#include <AUI/Thread/AThread.h>

template <typename ... Args>
void ASignal<Args...>::invokeSignal()
{
    if (mSlots.empty())
        return;
    std::unique_lock lock(mSlotsLock);
    for (auto i = mSlots.begin(); i != mSlots.end();)
    {
        if (i->object->getThread() != AThread::current())
        {
            auto obj = std::move(i->object);
            auto func = std::move(i->func);
            auto args = mArgs;
            obj->getThread()->enqueue([this, obj, func, args]() {
                AAbstractSignal::isDisconnected() = false;
                (std::apply)(func, args);
                if (AAbstractSignal::isDisconnected()) {
                    std::unique_lock lock(mSlotsLock);
                    unlinkSlot(obj);
                }
            });
            i = mSlots.erase(i);
        }
        else
        {
            AAbstractSignal::isDisconnected() = false;
            (std::apply)(i->func, mArgs);
            if (AAbstractSignal::isDisconnected()) {
                unlinkSlot(i->object);
                i = mSlots.erase(i);
                continue;
            }
            ++i;
        }
    }
}

template<typename... Args>
using emits = ASignal<Args...>;

#define signals public