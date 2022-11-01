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
#include "AUI/Common/ASignal.h"

/**
 * @brief Stores a value and observes it's changes, emitting a signal.
 * @ingroup core
 */
template <typename T>
class AFieldSignalEmitter
{
public:
	using signal_t = ASignal<T>&;
private:
	signal_t mSignal;
	T mValue;

public:
	AFieldSignalEmitter(signal_t signal, T initial = T()):
		mSignal(signal), mValue(std::move(initial))
	{
		
	}

	AFieldSignalEmitter& operator=(T t)
	{
		if (mValue != t) {
			mValue = std::move(t);
			mSignal(mValue);
		}
		return *this;
	}
	operator T() const
	{
		return mValue;
	}
};

template <>
class AFieldSignalEmitter<bool>
{
	using T = bool;
public:
	using signal_t = ASignal<T> &;
	using signal_sub_t = ASignal<> &;
private:
	signal_t mSignal;
	signal_sub_t mSignalTrue;
	signal_sub_t mSignalFalse;
	T mValue;

public:
	AFieldSignalEmitter(signal_t signal, signal_sub_t signalTrue, signal_sub_t signalFalse, T initial = false)
		: mSignal(signal),
		  mSignalTrue(signalTrue),
		  mSignalFalse(signalFalse),
		  mValue(initial)
	{
	}

	AFieldSignalEmitter& set(AObject* emitter, const T& t)
	{
		if (mValue != t) {
			mValue = t;
			mSignal(t).invokeSignal(emitter);
			if (t)
			{
				mSignalTrue().invokeSignal(emitter);
			}
			else
			{
				mSignalFalse().invokeSignal(emitter);
			}
		}
		return *this;
	}
	operator bool() const
	{
		return mValue;
	}
};