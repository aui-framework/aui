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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include "SharedPtrTypes.h"
#include "AUI/Core.h"
#include "AUI/Common/ASet.h"
#include <AUI/Thread/AMutex.h>

class AString;
class AAbstractSignal;
class AAbstractThread;

class API_AUI_CORE AObject
{
	friend class AAbstractSignal;
private:
	_<AAbstractThread> mAttachedThread;
    AMutex mSignalsLock;
    ASet<AAbstractSignal*> mSignals;
	bool mSignalsEnabled = true;

	static AObject*& sender_impl();
	static bool& isDisconnected();

protected:
	static void disconnect();

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
	static void connect(Signal& signal, Object object, Function function)
	{
		static_assert(std::is_base_of_v<AObject, typename std::remove_pointer<Object>::type>, "the passed object should be a base of the AObject class (use class YourObject: public AObject)");
		static_assert(std::is_pointer_v<Object>, "the object should be a pointer (use &yourObject)");

		if constexpr (std::is_base_of_v<AObject, typename std::remove_pointer<Object>::type> && std::is_pointer_v<Object>) {
			signal.connect(object, function);
		}
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

		if constexpr (std::is_base_of_v<AObject, typename std::remove_pointer<Object>::type>) {
			signal.connect(object.get(), function);
		}
	}

	void setSignalsEnabled(bool enabled)
	{
		mSignalsEnabled = enabled;
	}

    template<typename T>
	void operator^(T& t) {
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