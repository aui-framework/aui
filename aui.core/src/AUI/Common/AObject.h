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

#include "AObjectBase.h"

namespace aui::detail {
  template<typename Object, typename Lambda>
  Lambda&& makeLambda(Object*, Lambda&& lambda) requires requires { std::is_class_v<Lambda>; } {
    return std::forward<Lambda>(lambda);
  }

  template<typename Object1, typename Object2, typename Returns, typename... Args>
  auto makeLambda(Object1* object, Returns(Object2::*method)(Args...)) {
    return [object, method](Args... args) {
      (object->*method)(std::forward<Args>(args)...);
    };
  }
}

/**
 * @brief A base object class.
 * @ingroup core signal_slot
 * @details
 * AObject is required to use @ref signal_slot "signal-slot system".
 *
 * AObject keeps reference to itself via std::enable_shared_from_this. It can be accessed with sharedPtr() and weakPtr()
 * functions.
 */
class API_AUI_CORE AObject: public AObjectBase, public std::enable_shared_from_this<AObject> {
    friend class AAbstractSignal;
    template <typename ... Args>
    friend class ASignal;

public:
    AObject();
    virtual ~AObject() = default;

    static void disconnect();

    [[nodiscard]] _<AObject> sharedPtr() { return std::enable_shared_from_this<AObject>::shared_from_this(); }

    [[nodiscard]] _weak<AObject> weakPtr() { return std::enable_shared_from_this<AObject>::weak_from_this(); }

    /**
     * @brief Connects signal to the slot of the specified object.
     * @ingroup signal_slot
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @code{cpp}
     * connect(view->clicked, slot(otherObjectRawPtr)::handleButtonClicked);
     * @endcode
     * @param signal signal
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     */
    template <AAnySignal Signal, aui::derived_from<AObjectBase> Object, ACompatibleSlotFor<Signal> Function>
    static void connect(const Signal& signal, Object* object, Function&& function) {
        const_cast<Signal&>(signal).connect(object, aui::detail::makeLambda(object, std::forward<Function>(function)));
    }

    /**
     * @brief Connects property to the slot of the specified object.
     * @ingroup property_system
     * @details
     * Connects to "changed" signal of the property. Additionally, calls specified function with the current value of the
     * property (pre-fire).
     *
     * See @ref signal_slot "signal-slot system" for more info.
     * @code{cpp}
     * connect(textField->text(), slot(otherObjectRawPtr)::handleText);
     * @endcode
     * @param property property
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     */
    template <AAnyProperty Property, aui::derived_from<AObjectBase> Object, typename Function>
    static void connect(const Property& property, Object* object, Function&& function) {
        auto lambda = aui::detail::makeLambda(object, std::forward<Function>(function));
        property.changed.makeRawInvocable(lambda)(*property);
        connect(property.changed, object, std::move(lambda));
    }

    /**
     * @brief Connects source property to the destination property.
     * @ingroup property_system
     * @details
     * Connects \c propertySource.changed to the setter of \c propertyDestination . Additionally, sets the
     * \c propertyDestination with the current value of the \c propertySource (pre-fire). Hence, dataflow is from left
     * argument to the right argument.
     *
     * connect pulls AObject from \c propertyDestination to maintain the connection.
     *
     * See @ref signal_slot "signal-slot system" for more info.
     * @param propertySource source property, whose value is preserved on connection creation.
     * @param propertyDestination destination property, whose value is overwritten on connection creation.
     */
    template <APropertyReadable PropertySource, APropertyWritable PropertyDestination>
    static void connect(PropertySource&& propertySource, PropertyDestination&& propertyDestination) requires requires {
            // source and destination properties must have compatible underlying types
            { *propertySource } -> aui::convertible_to<std::decay_t<decltype(*propertyDestination)>>;
        } {
        AObject::connect(propertySource,
                         propertyDestination.assignment());
    }

    /**
     * @brief Connects source property to the destination property and opposite (bidirectionally).
     * @ingroup property_system
     * @details
     * Connects \c propertySource.changed to the setter of \c propertyDestination . Additionally, sets the
     * \c propertyDestination with the current value of the \c propertySource (pre-fire). Hence, initial dataflow is
     * from left argument to the right argument.
     *
     * After pre-fire, connects \c propertyDestination.changed to the setter of \c propertySource . This way, when
     * \c propertyDestination changes (i.e, \c propertyDestination belongs to some view and it's value is changed due to
     * user action) it immediately reflects on \c propertySource . So, \c propertySource is typically a property of some
     * view model with prefilled interesting data, and propertyDestination is a property of some view whose value
     * is unimportant at the moment of connection creation.
     *
     * biConnect pulls AObject from \c propertySource and \c propertyDestination to maintain the connection.
     *
     * See @ref signal_slot "signal-slot system" for more info.
     * @param propertySource source property, whose value is preserved on connection creation.
     * @param propertyDestination destination property, whose value is overwritten on connection creation.
     */
    template <APropertyWritable PropertySource, APropertyWritable PropertyDestination>
    static void biConnect(PropertySource&& propertySource, PropertyDestination&& propertyDestination) requires requires {
            // source and destination properties must have compatible underlying types
            { *propertySource } -> aui::convertible_to<std::decay_t<decltype(*propertyDestination)>>;
            { *propertyDestination } -> aui::convertible_to<std::decay_t<decltype(*propertySource)>>;
        } {
        AObject::connect(propertySource,
                         propertyDestination.assignment());
        AObject::connect(propertyDestination.changed,
                         propertySource.assignment());
    }

    /**
     * @brief Connects signal or property to the slot of the specified object.
     * @ingroup signal_slot
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @code{cpp}
     * connect(view->clicked, slot(otherObjectRef)::handleButtonClicked);
     * @endcode
     * @param connectable signal or property
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     */
    template <AAnySignalOrProperty Connectable, aui::derived_from<AObjectBase> Object,
        ACompatibleSlotFor<Connectable> Function>
    static void
    connect(const Connectable& connectable, Object& object, Function&& function) {
        connect(connectable, &object, std::forward<Function>(function));
    }

    /**
     * @brief Connects signal or property to slot of \c "this" object.
     * @ingroup signal_slot
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @code{cpp}
     * connect(view->clicked, [] { printf("Button clicked!\\n"); });
     * connect(textField->text(), [](const AString& s) { ALogger::info(LOG_TAG) << "Text: " << s; });
     * @endcode
     * @param connectable signal or property
     * @param function slot. Can be lambda
     */
    template <typename Connectable, ACompatibleSlotFor<Connectable> Function>
    void connect(const Connectable& connectable, Function&& function) {
        connect(connectable, this, std::forward<Function>(function));
    }

    /**
     * @brief Connects signal or property to the slot of the specified object.
     * @ingroup signal_slot
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @code{cpp}
     * connect(view->clicked, slot(otherObjectSharedPtr)::handleButtonClicked);
     * connect(textField->text(), slot(otherObjectSharedPtr)::handleText);
     * @endcode
     * @note
     * `object` arg is accepted by value intentionally -- this way we ensure that it would not be destroyed during
     * connection creation.
     *
     * @param connectable signal or property
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     */
    template <AAnySignalOrProperty Connectable, aui::derived_from<AObjectBase> Object, ACompatibleSlotFor<Connectable> Function>
    static void
    connect(const Connectable& connectable, _<Object> object, Function&& function) {
        connect(connectable, object.get(), std::forward<Function>(function));
    }

    /**
     * @brief Connects signal to the slot of the specified object. Slot is packed to single argument.
     * @param connectable signal or property
     * @param slotDef instance of <code>AObject</code> + slot
     *
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @code{cpp}
     * connect(view->clicked, ASlotDef { slot(otherObject)::handleButtonClicked });
     * connect(textField->text(), ASlotDef { slot(otherObject)::handleText });
     * @endcode
     * @note
     * This overload is applicable for cases when you NEED to pass object and its slot via single argument. If possible,
     * consider using shorter overload:
     * @code{cpp}
     * connect(view->clicked, slot(otherObject)::handleButtonClicked);
     * @endcode
     */
    template <AAnySignalOrProperty Connectable, aui::derived_from<AObjectBase> Object, typename Function>
    static void
    connect(const Connectable& connectable, ASlotDef<Object*, Function> slotDef) {
        connect(connectable, slotDef.boundObject, std::move(slotDef.invocable));
    }

    /**
     * @brief Connects signal or property to the slot of the specified non-AObject type.
     * @ingroup property_system
     * @details
     * See @ref signal_slot "signal-slot system" for more info.
     * @code{cpp}
     * struct User { AProperty<AString> name }; // user.name here is non-AObject type
     * connect(textField->text(), user->name.assignment());
     * @endcode
     * @note
     * `object` arg is accepted by value intentionally -- this way we ensure that it would not be destroyed during
     * connection creation.
     *
     * @param property source property.
     * @param object instance of `AObject`.
     * @param function slot. Can be lambda.
     */
    template <AAnyProperty Property, typename Object, ACompatibleSlotFor<Property> Function>
    static void
    connect(const Property& property, _<Object> object, Function&& function)
        requires (!aui::derived_from<Object, AObject>)
    {
        property.changed.makeRawInvocable(function)(*property);
        connect(property.changed, object, std::forward<Function>(function));
        const_cast<std::decay_t<decltype(property.changed)>&>(property.changed).connectNonAObject(std::move(object), aui::detail::makeLambda(object,std::forward<Function>(function)));
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
    bool mSignalsEnabled = true;

    /**
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
