#pragma once

#include <atomic>
#include <functional>
#include "AConditionVariable.h"
#include "AMutex.h"

class AThreadPool;

template<typename Value>
class AFuture
{
private:
	Value* mValue = new Value;
	AMutex mMutex;
	AConditionVariable mNotify;
	std::atomic_int mRefCount = 2;

	void decRef()
	{
		if (--mRefCount == 0)
		{
			delete mValue;
		}
	}
	
public:

	template<typename Callable>
	AFuture(AThreadPool& tp, Callable func);

	Value& operator*() {
		if (mRefCount == 2) {
			std::unique_lock lock(mMutex);
			while (mRefCount == 2)
			{
				mNotify.wait(lock);
			}
		}
		return *mValue;
	}

	inline Value& get()
	{
		return **this;
	}

	~AFuture()
	{
		decRef();
	}
};

#include <AUI/Thread/AThreadPool.h>

template <typename Value>
template <typename Callable>
AFuture<Value>::AFuture(AThreadPool& tp, Callable func)
{
	tp.run([&, func]()
	{
		std::unique_lock lock(mMutex);
		*mValue = func();
		decRef();
		mNotify.notify_one();
	});
}
