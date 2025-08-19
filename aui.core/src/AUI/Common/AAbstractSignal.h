/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <exception>

#include <AUI/Core.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Traits/values.h>

class AObject;

/**
 * @defgroup signal_slot Signal-slot
 * @ingroup core
 * @brief Signal-slots is an object messaging mechanism that creates seamless relations between objects.
 * @details Signal-slots were originally implemented in Qt and it has proven themselves as an object messaging mechanism
 * making it easy to implement the observer pattern without boilerplate code.
 * Signal-slot tracks object existence on the both ends so destruction of either sender or receiver object breaks the
 * link between them. When the sender or receiver object is destroyed slot execution is never possible even in a
 * multithreaded environment. Almost any signal can be connected to almost any slot (even lambda) by any code.
 *
 * All classes that inherit from AObject or one of its subclasses (e.g., AView) can contain signals and slots. Signals
 * are emitted by objects when they change their state in a way that may be interesting to other objects. This is all
 * the object does to communicate. It does not know or care whether anything is receiving the signals it emits. This is
 * true information encapsulation, and ensures that the object can be used as a software component.
 *
 * [Signal declarations](emits) are special public fields of any class that inherit from AObject.
 * ```cpp
 * class Counter: public AObject {
 * public:
 *   Counter() = default;
 *
 *   void setValue(int value) {
 *     mValue = value;
 *     emit valueChanged;
 *   }
 *
 * signals:
 *   emits<> valueChanged;
 *
 * private:
 *   int mValue = 0;
 * };
 * ```
 *
 * They can specify arguments:
 * ```cpp
 * emits<int> valueChanged;
 * ...
 * emit valueChanged(mValue);
 * ```
 *
 * Any member function of a class can be used as a slot.
 *
 * You can connect as many signals as you want to a single slot, and a signal can be connected to as many slots as you
 * need.
 *
 * ## Signals
 * Signals are publicly accessible fields that notify an object's client when its internal state has changed in some way
 * that might be interesting or significant. These signals can be emitted from various locations, but it is generally
 * recommended to only emit them from within the class that defines the signal and its subclasses.
 *
 * The slots connected to a signal are generaly executed immediately when the signal is emitted like a regular function
 * call. \c emit returns control after all slots are called. If receiver has AObject::setSlotsCallsOnlyOnMyThread set to
 * true and \c emit was called on a different thread, \c emit queues slot execution to the AEventLoop associated with
 * receiver's thread. All AView have this behaviour enabled by default.
 *
 * ## Slots
 * A slot is triggered by the emission of a related signal. Slot is no more than a regular class method with a signal
 * connected to it.
 *
 * When called directly, slots follow standard C++ rules and syntax. However, through a signal-slot connection, any
 * component can invoke a slot regardless of its accessibility level. Visibility scope matters only when creating
 * connection (AObject::connect). This means a signal from one class can call a private slot in another unrelated class
 * if connection is created within class itself (with access to its private/protected methods).
 *
 * Furthermore, slots can be defined as virtual functions, which have been found to be beneficial in practical
 * application development.
 *
 * Compared to callbacks, the signals and slots system has some performance overhead due to its increased flexibility.
 * The difference is typically insignificant for real-world applications. In general, emitting a signal connected to
 * various slots can result in an execution time roughly ten times slower than direct function calls. This delay comes
 * from locating the connection object and safely iterating over connections.
 *
 * The trade-off between the signals and slots mechanism's simplicity and flexibility compared to pure function calls or
 * callbacks is well-justified for most applications, given its minimal performance cost that users won't perceive.
 *
 * ## Basic example
 * Let's use \c Counter from our previous example:
 * ```cpp
 * class Counter: public AObject {
 * public:
 *   Counter() = default;
 *
 *   void setValue(int value) {
 *     mValue = value;
 *     emit valueChanged(value);
 *   }
 *
 * signals:
 *   emits<int> valueChanged;
 *
 * private:
 *   int mValue = 0;
 * };
 *
 * class MyApp: public AObject {
 * public:
 *   MyApp() {}
 *
 *   void run() {
 *     connect(mCounter->valueChanged, this, &MyApp::printCounter);
 *     mCounter->setValue(123); // prints "New value: 123"
 *   }
 *
 * private:
 *   _<Counter> mCounter = _new<Counter>();
 *
 *   void printCounter(int value) {
 *     ALogger::info("MyApp") << value;
 *   }
 * };
 *
 * AUI_ENTRY {
 *   auto app = _new<MyApp>();
 *   app->run();
 *   return 0;
 * }
 * ```
 *
 * If `connect(mCounter->valueChanged, this, &MyApp::printCounter);` looks too long for you, you can use
 * [AUI_SLOT](AUI_SLOT) macro:
 * ```cpp
 * connect(mCounter->valueChanged, AUI_SLOT(this)::printCounter);
 * ```
 *
 * Furthermore, when connecting to `this`, AUI_SLOT(this) can be replaced with [me](me):
 * ```cpp
 * connect(mCounter->valueChanged, me::printCounter);
 * ```
 *
 * Lambda can be used as a slot either:
 * ```cpp
 * connect(mCounter->valueChanged, this, [](int value) {
 *   ALogger::info("MyApp") << value;
 * });
 * ```
 *
 * As with methods, `this` can be omitted:
 * ```cpp
 * connect(mCounter->valueChanged, [](int value) {
 *   ALogger::info("MyApp") << value;
 * });
 * ```
 *
 *
 * ## UI example
 * Knowing basics of signal slots, you can now utilize UI signals:
 * ```cpp
 * mOkButton = _new<AButton>("OK");
 * ...
 * connect(mOkButton->clicked, [] { // the signal is connected to "this" object
 *     ALogger::info("Example") << "The button was clicked";
 * });
 * ```
 *
 * !!! bug "Pitfall"
 *
 *     In lambda, do not capture shared pointer (AUI's _) of signal emitter or receiver object by value. This would cause
 *     a memory leak:
 *     ```
 *     mOkButton = _new<AButton>("OK");
 *     ...
 *     connect(mOkButton->clicked, [view] { // WRONG!!!
 *         view->setText("clicked");
 *     });
 *     ```
 *     Do this way:
 *     ```
 *     mOkButton = _new<AButton>("OK");
 *     ...
 *     connect(mOkButton->clicked, [view = view.get()] { // ok
 *         view->setText("clicked");
 *     });
 *     ```
 *
 * ### Going further
 * Let's take our previous example with `Counter` and make an UI app. Signal slot reveals it's power when your objects
 * have small handy functions, so lets add `increase` method to our counter:
 *
 * ```cpp
 * class Counter: public AObject {
 * public:
 *   Counter() = default;
 *
 *   void setValue(int value) {
 *     mValue = value;
 *     emit valueChanged(value);
 *   }
 *
 *   void increase() {
 *     setValue(mCounter + 1);
 *   }
 *
 * signals:
 *   emits<int> valueChanged;
 *
 * private:
 *   int mValue = 0;
 * };
 *
 * class MyApp: public AWindow {
 * public:
 *   MyApp() {
 *     auto label = _new<ALabel>("-");
 *     auto button = _new<AButton>("Increase");
 *
 *     using namespace declarative;
 *     setContents(Vertical {
 *       label,
 *       button,
 *     });
 *
 *     connect(button->clicked, AUI_SLOT(mCounter)::increase); // beauty, huh?
 *     connect(mCounter->valueChanged, label, [label = label.get()](int value) {
 *       label->setText("{}"_format(value));
 *     });
 *   }
 *
 * private:
 *   _<Counter> mCounter = _new<Counter>();
 * };
 *
 * AUI_ENTRY {
 *   auto app = _new<MyApp>();
 *   app->show();
 *   return 0;
 * }
 * ```
 *
 * This way, by clicking on "Increase", it would increase the counter and immediately display value via label.
 *
 * Let's make things more declarative and use [AUI_LET](AUI_LET) syntax to set up connections:
 * ```cpp
 * MyApp() {
 *   using namespace declarative;
 *   setContents(Vertical {
 *     _new<ALabel>("-") AUI_LET {
 *       connect(counter->valueChanged, label, [label = it.get()](int value) {
 *         label->setText("{}"_format(value));
 *       });
 *     },
 *     _new<AButton>("Increase") AUI_LET {
 *       connect(it->clicked, AUI_SLOT(mCounter)::increase);
 *     },
 *   });
 * }
 * ```
 *
 * See also [property-system] for making reactive UI's on trivial data.
 *
 * ## Arguments
 * If signal declares arguments (i.e, like AView::keyPressed), you can accept them:
 * ```cpp
 * view = _new<ATextField>();
 * ...
 * connect(view->keyPressed, [](AInput::Key k) { // the signal is connected to "this" object
 *     ALogger::info("Example") << "Key was pressed: " << k;
 * });
 * ```
 *
 * The signals and slots mechanism is type safe: The signature of a signal must match the signature of the receiving
 * slot. Also, a slot may have a shorter signature than the signal it receives because it can ignore extra arguments:
 * ```cpp
 * view = _new<ATextField>();
 * ...
 * connect(view->keyPressed, [] { // the signal is connected to "this" object
 *     ALogger::info("Example") << "Key was pressed";
 * });
 * ```
 *
 * ## Differences between Qt and AUI implementation
 * Suppose we want to emit <code>statusChanged</code> signal with a string argument and connect it with
 * <code>showMessage</code> slot:
 * <table>
 *   <tr>
 *     <th></th>
 *     <th>Qt</th>
 *     <th>AUI</th>
 *   </tr>
 *   <tr>
 *     <td>Signal declaration</td>
 *     <td>
 *     ```cpp
 *     signals:
 *       void statusChanged(QString str);
 *     ```
 *     </td>
 *     <td>
 *     ```cpp
 *     signals:
 *       emits<AString> statusChanged;
 *     ```
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>Slot declaration</td>
 *     <td>
 *     ```cpp
 *     slots:
 *       void showMessage(QString str);
 *     ```
 *     </td>
 *     <td>
 *     ```cpp
 *       void showMessage(AString str);
 *     ```
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>Connect from <code>this</code> to <code>this</code></td>
 *     <td>
 *     ```cpp
 *     connect(this, SIGNAL(statusChanged(QString), this, SLOT(showMessage(QString)));
 *     ```
 *     </td>
 *     <td>
 *     ```cpp
 *       connect(statusChanged, me::showMessage);
 *     ```
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>Connect from the <code>emitter</code> object to the <code>sender</code> object</td>
 *     <td>
 *     ```cpp
 *      QObject::connect(emitter, SIGNAL(statusChanged(QString), receiver, SLOT(showMessage(QString)));
 *     ```
 *     </td>
 *     <td>
 *     ```cpp
 *       AObject::connect(emitter->statusChanged, AUI_SLOT(receiver)::showMessage);
 *     ```
 *     </td>
 *   </tr>
 * </table>
 *
 */

/**
 * @brief Base class for signal.
 * @details Since ASignal is a template class, AAbstractSignal provides unified access to template inseparable fields of
 * ASignal.
 * @ingroup core
 * @ingroup signal_slot
 */
class API_AUI_CORE AAbstractSignal {
    friend class AObject;

public:
    virtual ~AAbstractSignal() = default;

    /**
     * @brief Connection handle.
     */
    struct Connection {
        friend class API_AUI_CORE AObjectBase;
        /**
         * @brief Breaks connection.
         */
        virtual void disconnect() = 0;

    private:
        /**
         * @brief Breaks connection in the sender side.
         * @details
         * Called when `AObjectBase` has cleaned its connection instance.
         *
         * This cleanup function assumes that an appropriate clean action for the receiver side is taken.
         */
        virtual void onBeforeReceiverSideDestroyed() = 0;
    };

    /**
     * @brief Connection owner which destroys the connection in destructor.
     */
    struct AutoDestroyedConnection {
        _<Connection> value = nullptr;

        AutoDestroyedConnection() = default;
        AutoDestroyedConnection(_<Connection> connection) noexcept : value(std::move(connection)) {}
        AutoDestroyedConnection(const AutoDestroyedConnection&) = default;
        AutoDestroyedConnection(AutoDestroyedConnection&&) noexcept = default;

        AutoDestroyedConnection& operator=(_<Connection> rhs) noexcept {
            if (value == rhs) {
                return *this;
            }
            release();
            value = std::move(rhs);
            return *this;
        }

        AutoDestroyedConnection& operator=(const AutoDestroyedConnection& rhs) {
            return operator=(rhs.value);
        }

        AutoDestroyedConnection& operator=(AutoDestroyedConnection&& rhs) noexcept {
            return operator=(std::move(rhs.value));
        }

        ~AutoDestroyedConnection() {
            release();
        }

    private:
        void release() noexcept {
            if (!value) {
                return;
            }
            value->disconnect();
        }
    };

    /**
     * @brief Destroys all connections of this signal, if any.
     */
    virtual void clearAllOutgoingConnections() const noexcept = 0;

    /**
     * @brief Destroys all connections with passed receiver, if any.
     * @param receiver object to clear connections with.
     */
    virtual void clearAllOutgoingConnectionsWith(aui::no_escape<AObjectBase> receiver) const noexcept = 0;

    /**
     * @param receiver receiver objects to check connections with.
     * @return Whether this signal has connections with passed receiver object.
     */
    virtual bool hasOutgoingConnectionsWith(aui::no_escape<AObjectBase> receiver) const noexcept = 0;

    /**
     * @brief Creates generic connection (without arguments).
     * @param receiver receiver object.
     * @param observer function to be called when signal is fired.
     */
    virtual _<Connection> addGenericObserver(AObjectBase* receiver, std::function<void()> observer) = 0;

protected:
    /* some handy functions accessed from public headers */

    static _weak<AObject> weakPtrFromObject(AObject* object);

    /**
     * @brief Adds a connection to the specified object.
     */
    static void addIngoingConnectionIn(aui::no_escape<AObjectBase> object, _<Connection> connection);

    /**
     * @brief Removes a connection from the specified object.
     */
    static void removeIngoingConnectionIn(aui::no_escape<AObjectBase> object, Connection& connection, std::unique_lock<ASpinlockMutex>& lock);
};
