// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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