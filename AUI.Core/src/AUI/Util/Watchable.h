#pragma once
#include "AUI/Common/ASignal.h"

template <typename T>
class Watchable
{
public:
	using signal_t = ASignal<T>&;
private:
	signal_t mSignal;
	T mValue;

public:
	Watchable(signal_t signal, T initial = T()):
		mSignal(signal), mValue(initial)
	{
		
	}

	Watchable& operator=(const T& t)
	{
		if (mValue != t) {
			mValue = t;
			mSignal(t);
		}
		return *this;
	}
	operator T() const
	{
		return mValue;
	}
};

template <>
class Watchable<bool>
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
	Watchable(signal_t signal, signal_sub_t signalTrue, signal_sub_t signalFalse, T initial = false)
		: mSignal(signal),
		  mSignalTrue(signalTrue),
		  mSignalFalse(signalFalse),
		  mValue(initial)
	{
	}

	Watchable& operator=(const T& t)
	{
		if (mValue != t) {
			mValue = t;
			mSignal(t).invokeSignal();
			if (t)
			{
				mSignalTrue().invokeSignal();
			}
			else
			{
				mSignalFalse().invokeSignal();
			}
		}
		return *this;
	}
	operator bool() const
	{
		return mValue;
	}
};