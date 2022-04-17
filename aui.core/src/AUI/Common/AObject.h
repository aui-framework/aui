﻿/**
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

#include "SharedPtrTypes.h"
#include "AUI/Core.h"
#include "AUI/Common/ASet.h"
#include "AUI/Traits/values.h"
#include <AUI/Thread/AMutex.h>

class AString;
class AAbstractSignal;
class AAbstractThread;

class API_AUI_CORE AObject: public aui::noncopyable, public std::enable_shared_from_this<AObject>
{
	friend class AAbstractSignal;
private:
	_<AAbstractThread> mAttachedThread;
    AMutex mSignalsLock;
    ASet<AAbstractSignal*> mSignals;
	bool mSignalsEnabled = true;

	static bool& isDisconnected();

protected:
	static void disconnect();

    [[nodiscard]]
    _<AObject> objectSharedPtr() {
        return std::enable_shared_from_this<AObject>::shared_from_this();
    }

    [[nodiscard]]
    _weak<AObject> objectWeakPtr() {
        return std::enable_shared_from_this<AObject>::weak_from_this();
    }

public:
	AObject();
	virtual ~AObject();

	void clearSignals();

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
	        t.invokeSignal();
	    }
	}


    _<AAbstractThread> getThread() const {
        return mAttachedThread;
    }
};

#define emit (*this)^

#include "SharedPtr.h"