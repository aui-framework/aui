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

#include <exception>

#include <AUI/Core.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Traits/values.h>

class AObject;

/**
 * @defgroup signal_slot Signal-slot
 * @ingroup core
 * @brief Signal-slots is an object messaging mechanism that creates seamless relations between objects.
 * @details Signal-slots originally implemented in Qt and it has proven itself as an object messaging mechanism making
 * it easy to implement the observer pattern without boilerplate code.
 * Signal-slot tracks object existence on the both ends so destruction of either sender or receiver object breaks the
 * link between them. When the sender or receiver object is destroyed slot execution is never possible even in a
 * multithreaded environment. Almost any signal can be connected to almost any slot (even lambda) by any code.
 *
 * <h1>Basic example</h1>
 * @code{cpp}
 * mOkButton = _new<AButton>("OK");
 * ...
 * connect(mOkButton->clicked, [] { // the signal is connected to "this" object
 *     ALogger::info("The button was clicked");
 * });
 * @endcode
 *
 * <h1>Differences between Qt and AUI implementation</h1>
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
 *     @code{cpp}
 *     signals:
 *       void statusChanged(QString str);
 *     @endcode
 *     </td>
 *     <td>
 *     @code{cpp}
 *     signals:
 *       emits<AString> statusChanged;
 *     @endcode
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>Slot declaration</td>
 *     <td>
 *     @code{cpp}
 *     slots:
 *       void showMessage(QString str);
 *     @endcode
 *     </td>
 *     <td>
 *     @code{cpp}
 *       void showMessage(AString str);
 *     @endcode
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>Connect from <code>this</code> to <code>this</code></td>
 *     <td>
 *     @code{cpp}
 *     connect(this, SIGNAL(statusChanged(QString), this, SLOT(showMessage(QString)));
 *     @endcode
 *     </td>
 *     <td>
 *     @code{cpp}
 *       connect(statusChanged, me::showMessage);
 *     @endcode
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>Connect from the <code>emitter</code> object to the <code>sender</code> object</td>
 *     <td>
 *     @code{cpp}
 *      QObject::connect(emitter, SIGNAL(statusChanged(QString), receiver, SLOT(showMessage(QString)));
 *     @endcode
 *     </td>
 *     <td>
 *     @code{cpp}
 *       AObject::connect(emitter->statusChanged, slot(receiver)::showMessage);
 *     @endcode
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
class API_AUI_CORE AAbstractSignal
{
    friend class AObject;
public:
    virtual ~AAbstractSignal() {
        mDestroyed = true;
    }

    [[nodiscard]] bool isDestroyed() const noexcept {
        return mDestroyed;
    }

    virtual void clearAllConnectionsWith(aui::no_escape<AObject> object) noexcept = 0;
    virtual void clearAllConnections() noexcept = 0;

protected:
    void linkSlot(AObject* object) noexcept;
    void unlinkSlot(AObject* object) noexcept;

    static bool& isDisconnected();

    static _weak<AObject> weakPtrFromObject(AObject* object);

private:
    bool mDestroyed = false;
};

#include <AUI/Common/AObject.h>

inline bool& AAbstractSignal::isDisconnected() {
    return AObject::isDisconnected();
}

inline _weak<AObject> AAbstractSignal::weakPtrFromObject(AObject* object) { // AAbstractSignal is a friend of AObject
    return object->weakPtr();
}
