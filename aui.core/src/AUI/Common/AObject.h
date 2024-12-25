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

#include <AUI/Thread/AMutex.h>
#include <AUI/Traits/concepts.h>
#include <AUI/Traits/members.h>

#include "AUI/Common/ASet.h"
#include "AUI/Core.h"
#include "AUI/Traits/values.h"
#include "SharedPtrTypes.h"

/**
 * @brief A base object class.
 * @ingroup core signal_slot
 * @details
 * AObject is required to use @ref signal_slot "signal-slot system".
 *
 * AObject keeps reference to itself via std::enable_shared_from_this. It can be accessed with sharedPtr() and weakPtr()
 * functions.
 */
class API_AUI_CORE AObject : public aui::noncopyable, public std::enable_shared_from_this<AObject> {
    friend class AAbstractSignal;

public:
    AObject();
    virtual ~AObject();

    static void disconnect();

    [[nodiscard]] _<AObject> sharedPtr() { return std::enable_shared_from_this<AObject>::shared_from_this(); }

    [[nodiscard]] _weak<AObject> weakPtr() { return std::enable_shared_from_this<AObject>::weak_from_this(); }

    void clearSignals() noexcept;

    /**
     * @brief Connects signal to the slot of the specified object.
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @example
     * @code{cpp}
     * connect(view->clicked, slot(otherObject)::handleButtonClicked);
     * @endcode
     * @param signal signal
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     * @param projection projection
     */
    template <AAnySignal Signal, aui::derived_from<AObject> Object, ACompatibleSlotFor<Signal> Function, typename Projection = std::identity>
    static void connect(const Signal& signal, Object* object, Function&& function, Projection&& projection = {}) {
        const_cast<Signal&>(signal).connect(object, std::forward<Function>(function), std::forward<Projection>(projection));
    }

    /**
     * @brief Connects property to the slot of the specified object.
     * @details
     * Connects to "changed" signal of the property. Additionally, calls specified function with the current value of the
     * property (pre-fire).
     *
     * See @ref signal_slot "signal-slot system" for more info.
     * @example
     * @code{cpp}
     * connect(view->clicked, slot(otherObject)::handleButtonClicked);
     * @endcode
     * @param property property
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     * @param projection projection
     */
    template <AAnyProperty Property, aui::derived_from<AObject> Object, typename Function, typename Projection = std::identity>
    static void connect(const Property& property, Object* object, Function&& function, Projection&& projection = {}) {
        std::decay_t<decltype(property.changed)>::makeCallable(object, function, projection)(*property);
        connect(property.changed, object, std::forward<Function>(function), std::forward<Projection>(projection));
    }



    /**
     * @brief Connects signal or property to the slot of the specified object.
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @example
     * @code{cpp}
     * connect(view->clicked, slot(otherObject)::handleButtonClicked);
     * @endcode
     * @param connectable signal or property
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     * @param projection projection
     */
    template <
        typename Connectable, aui::derived_from<AObject> Object, ACompatibleSlotFor<Connectable> Function,
        typename Projection = std::identity>
    static void
    connect(const Connectable& connectable, Object& object, Function&& function, Projection&& projection = {})
        requires AAnySignal<Connectable> || AAnyProperty<Connectable>
    {
        connect(connectable, &object, std::forward<Function>(function), std::forward<Projection>(projection));
    }

    /**
     * @brief Connects signal or property to slot of \c "this" object.
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @example
     * @code{cpp}
     * connect(view->clicked, [] { printf("Button clicked!\\n"); });
     * @endcode
     * @param connectable signal or property
     * @param function slot. Can be lambda
     * @param projection projection
     * @param projection projection
     */
    template <typename Connectable, ACompatibleSlotFor<Connectable> Function, typename Projection = std::identity>
    void connect(const Connectable& connectable, Function&& function, Projection&& projection = {})
        requires AAnySignal<Connectable> || AAnyProperty<Connectable>
    {
        const_cast<Connectable&>(connectable)
            .connect(this, std::forward<Function>(function), std::forward<Projection>(projection));
    }

    /**
     * @brief Connects signal or property to the slot of the specified object.
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @example
     * @code{cpp}
     * connect(view->clicked, slot(otherObject)::handleButtonClicked);
     * @endcode
     * @param connectable signal or property
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     */
    template <
        typename Connectable, aui::derived_from<AObject> Object, ACompatibleSlotFor<Connectable> Function,
        typename Projection = std::identity>
    static void
    connect(const Connectable& connectable, _<Object> object, Function&& function, Projection&& projection = {})
        requires AAnySignal<Connectable> || AAnyProperty<Connectable>
    {
        connect(connectable, object.get(), std::forward<Function>(function), std::forward<Projection>(projection));
    }

    void setSignalsEnabled(bool enabled) { mSignalsEnabled = enabled; }

    [[nodiscard]] bool isSignalsEnabled() const noexcept { return mSignalsEnabled; }

    template <ASignalInvokable T>
    void operator^(T&& t) {
        if (mSignalsEnabled) {
            t.invokeSignal(this);
        }
    }

    _<AAbstractThread> getThread() const { return mAttachedThread; }

    bool isSlotsCallsOnlyOnMyThread() const noexcept { return mSlotsCallsOnlyOnMyThread; }

    static void moveToThread(aui::no_escape<AObject> object, _<AAbstractThread> thread);

    void setSlotsCallsOnlyOnMyThread(bool slotsCallsOnlyOnMyThread) {
        mSlotsCallsOnlyOnMyThread = slotsCallsOnlyOnMyThread;
    }

protected:
    /**
     * @brief Set thread of the object.
     */
    void setThread(_<AAbstractThread> thread) { mAttachedThread = std::move(thread); }

private:
    _<AAbstractThread> mAttachedThread;
    AMutex mSignalsLock;
    ASet<AAbstractSignal*> mSignals;
    bool mSignalsEnabled = true;

    /*
     * @brief Allows cross-thread signal call through event loop.
     * @details
     * If the object is sensitive to calls from other threads (i.e. view), it may set this flag to true in order to
     * force signals to pass through the object's native thread instead of calls from the other threads.
     */
    bool mSlotsCallsOnlyOnMyThread = false;

    static bool& isDisconnected();
};

/**
 * @brief emits the specified signal in context of \c this object.
 * @details
 * @ingroup useful_macros
 * @ingroup signal_slot
 * Unlike Qt's emit, AUI's emit is not just a syntax sugar; it's required to actually perform a signal call.
 *
 * Basic example: (in context of member function of AView):
 * @code{cpp}
 * emit clicked;
 * @endcode
 *
 * This code calls slots connected to <code>clicked</code> signal.
 *
 * If @ref emits "signal declaration" has arguments, you have to specify them in braces:
 * @code{cpp}
 * emit keyPressed(AInput::LCTRL);
 * @endcode
 *
 * See @ref signal_slot "signal-slot system" for more info.
 */
#define emit (*this) ^

/**
 * @brief emits the specified signal in context of specified object.
 * @param object pointer-like reference to object to emit signal from.
 * @param signal signal field name
 * @param ... arguments to the signal
 * @details
 * @ingroup useful_macros
 * @ingroup signal_slot
 * Unlike emit, this macro allows to emit signal of other object. It's recommended to use AUI_EMIT_FOREIGN only if
 * there's no way to use emit.
 *
 * Basic example: (in context of member function of AView):
 * @code{cpp}
 * auto view = _new<AButton>("button"); // or whatever view
 * AUI_EMIT_FOREIGN(view, clicked);
 * @endcode
 *
 * This code calls slots connected to <code>clicked</code> signal.
 *
 * If @ref emits "signal declaration" has arguments, you have to specify them:
 * @code{cpp}
 * AUI_EMIT_FOREIGN(view, keyPressed, AInput::LCTRL);
 * @endcode
 *
 * See @ref signal_slot "signal-slot system" for more info.
 */
#define AUI_EMIT_FOREIGN(object, signal, ...) (*object) ^ object->signal(__VA_ARGS__)

#include "SharedPtr.h"
