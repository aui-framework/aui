// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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

#include "SharedPtrTypes.h"
#include "AUI/Core.h"
#include "AUI/Common/ASet.h"
#include "AUI/Traits/values.h"
#include <AUI/Thread/AMutex.h>

class AString;
class AAbstractSignal;
class AAbstractThread;

/**
 * @brief A base object class.
 * @ingroup core
 * @details
 * AObject is required to use signal slot system.
 *
 * AObject keeps reference to itself via std::enable_shared_from_this. It can be accessed with sharedPtr() and weakPtr()
 * functions.
 */
class API_AUI_CORE AObject: public aui::noncopyable, public std::enable_shared_from_this<AObject>
{
	friend class AAbstractSignal;
public:
	AObject();
	virtual ~AObject();

    static void disconnect();

    [[nodiscard]]
    _<AObject> sharedPtr() {
        return std::enable_shared_from_this<AObject>::shared_from_this();
    }

    [[nodiscard]]
    _weak<AObject> weakPtr() {
        return std::enable_shared_from_this<AObject>::weak_from_this();
    }

	void clearSignals() noexcept;

    /**
     * Connects signal to the slot of the specified object.
     * @example
     * <code>
     * connect(view->clicked, slot(otherObject)::handleButtonClicked);
     * </code>
     * @param signal signal
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     */
	template<class Signal, class Object, typename Function>
	static void connect(Signal& signal, Object* object, Function function)
	{
		static_assert(std::is_base_of_v<AObject, Object>, "the passed object should be a base of the AObject class (use class YourObject: public AObject)");
		static_assert(std::is_base_of_v<AAbstractSignal, Signal>, "expected signal as first argument");

        signal.connect(object, function);
	}

    /**
     * Connects signal to the slot of the specified object.
     * @example
     * <code>
     * connect(view->clicked, slot(otherObject)::handleButtonClicked);
     * </code>
     * @param signal signal
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     */
	template<class Signal, class Object, typename Function>
	static void connect(Signal& signal, Object& object, Function function)
	{
		static_assert(std::is_base_of_v<AObject, Object>, "the passed object should be a base of the AObject class (use class YourObject: public AObject)");
		static_assert(std::is_base_of_v<AAbstractSignal, Signal>, "expected signal as first argument");

        signal.connect(&object, function);
	}

    /**
     * Connects signal to slot of <code>this</code> object.
     * @example
     * <code>
     * connect(view->clicked, [] { printf("Button clicked!\\n"); });
     * </code>
     * @param signal signal
     * @param function slot. Can be lambda
     */
	template<class Signal, typename Function>
	void connect(Signal& signal, Function function)
	{
		static_assert(std::is_base_of_v<AAbstractSignal, Signal>, "expected signal as first argument");
		signal.connect(this, function);
	}

    /**
     * Connects signal to the slot of the specified object.
     * @example
     * <code>
     * connect(view->clicked, slot(otherObject)::handleButtonClicked);
     * </code>
     * @param signal signal
     * @param object instance of <code>AObject</code>
     * @param function slot. Can be lambda
     */
	template<class Signal, class Object, typename Function>
	static void connect(Signal& signal, _<Object> object, Function function)
	{
		static_assert(std::is_base_of_v<AObject, typename std::remove_pointer<Object>::type>, "the passed object should be a base of the AObject class (use class YourObject: public AObject)");
		static_assert(std::is_base_of_v<AAbstractSignal, Signal>, "expected signal as first argument");
		if constexpr (std::is_base_of_v<AObject, typename std::remove_pointer<Object>::type>) {
			signal.connect(object.get(), function);
		}
	}

	void setSignalsEnabled(bool enabled)
	{
		mSignalsEnabled = enabled;
	}

    template<typename T>
	void operator^(T&& t) {
	    if (mSignalsEnabled) {
	        t.invokeSignal(this);
	    }
	}


    _<AAbstractThread> getThread() const {
        return mAttachedThread;
    }

    bool isSlotsCallsOnlyOnMyThread() const noexcept {
        return mSlotsCallsOnlyOnMyThread;
    }

    static void moveToThread(aui::no_escape<AObject> object, _<AAbstractThread> thread);

protected:
    void setSlotsCallsOnlyOnMyThread(bool slotsCallsOnlyOnMyThread) {
        mSlotsCallsOnlyOnMyThread = slotsCallsOnlyOnMyThread;
    }

    /**
     * @brief Set thread of the object.
     */
    void setThread(_<AAbstractThread> thread) {
        mAttachedThread = std::move(thread);
    }

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

#define emit (*this)^

#include "SharedPtr.h"