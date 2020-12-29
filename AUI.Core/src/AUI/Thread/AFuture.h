#pragma once

#include <atomic>
#include <functional>
#include "AConditionVariable.h"
#include "AMutex.h"
#include <AUI/Common/SharedPtrTypes.h>

class AThreadPool;

template<typename Value>
class AFuture
{
private:
	Value* mValue = new Value;
	AMutex mMutex;
	AConditionVariable mNotify;
	std::function<void(const Value&)> mOnDone;
	std::atomic_int mRefCount = 2;

	void decRef()
	{
		if (--mRefCount == 0)
		{
			delete mValue;
		}
	}

	void notify() {
        mNotify.notify_one();
        if (mOnDone) {
            mOnDone(*mValue);
        }
	}


    AFuture() {}

public:

    template<typename Callable>
    static _<AFuture> make(AThreadPool& tp, Callable func);

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

    template<typename Object, typename Member>
	void onDone(const _<Object>& object, Member memberFunc) {
        std::unique_lock lock(mMutex);
	    mOnDone = [object, memberFunc](const Value& t) {
            (object.get()->*memberFunc)(t);
	    };
	}

    template<typename Object, typename Member>
	void onDone(const Object* object, Member memberFunc) {
        std::unique_lock lock(mMutex);
	    mOnDone = [object, memberFunc](const Value& t) {
            (object->*memberFunc)(t);
	    };
	}
    template<typename Callback>
	void onDone(const Callback& callable) {
        std::unique_lock lock(mMutex);
	    mOnDone = [callable](const Value& t) {
            callable(t);
	    };
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
_<AFuture<Value>> AFuture<Value>::make(AThreadPool& tp, Callable func)
{
    auto future = _<AFuture<Value>>(new AFuture<Value>);
	tp.run([future, func]()
	{
		std::unique_lock lock(future->mMutex);
		*future->mValue = func();
        future->decRef();
        future->notify();
	});
	return future;
}
