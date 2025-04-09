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