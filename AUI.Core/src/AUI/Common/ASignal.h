#pragma once

#include <functional>
#include "AUI/Common/ADeque.h"
#include "AUI/Thread/AMutex.h"
#include "AAbstractSignal.h"

template<typename... Args>
class ASignal: public AAbstractSignal
{
	friend class AObject;

	template <typename T>
	friend class Watchable;
public:
	using func_t = std::function<void(Args...)>;

private:
    std::tuple<Args...> mArgs;

	struct slot
	{
		AObject* object; // TODO сделать weak_ptr
		func_t func;
	};

	AMutex mSlotsLock;
	ADeque<slot> mSlots;

	void invokeSignal();

	template<typename Lambda, typename... A>
	struct call_helper {};

	// empty arguments
	template<typename Lambda>
	struct call_helper<void(Lambda::*)() const>
	{
		Lambda l;

		explicit call_helper(Lambda l)
			: l(l)
		{
		}

		void operator()(Args... args) {
			l();
		}
	};
	
	template<typename Lambda, typename A1>
	struct call_helper<void(Lambda::*)(A1) const>
	{
		Lambda l;

		explicit call_helper(Lambda l)
			: l(l)
		{
		}

		template<typename... Others>
		void call(A1 a1, Others...)
		{
			l(a1);
		}

		void operator()(Args... args) {
			call(args...);
		}
	};
	template<typename Lambda, typename A1, typename A2>
	struct call_helper<void(Lambda::*)(A1, A2) const>
	{
		Lambda l;

		explicit call_helper(Lambda l)
			: l(l)
		{
		}

		template<typename... Others>
		void call(A1 a1, A2 a2, Others...)
		{
			l(a1, a2);
		}

		void operator()(Args... args) {
			call(args...);
		}
	};
	
	
	
	// Member function
	template<class Derived, class Object, typename... FArgs>
	void connect(Derived derived, void(Object::* memberFunction)(FArgs...))
	{
		Object* object = static_cast<Object*>(derived);
		connect(object, [object, memberFunction](FArgs... args)
		{
			(object->*memberFunction)(args...);
		});
	}
	
	// Lambda function
	template<class Object, class Lambda>
	void connect(Object object, Lambda lambda)
	{
		static_assert(std::is_class_v<Lambda>, "the lambda should be a class");

		std::unique_lock lock(mSlotsLock);
		mSlots.push_back({ object, call_helper<decltype(&Lambda::operator())>(lambda) });

		linkSlot(object);
	}
	
public:
    ASignal<Args...>& operator()(const Args&... args) {
        mArgs = std::make_tuple(args...);
        return *this;
    }

	virtual ~ASignal()
	{
		std::unique_lock lock(mSlotsLock);
		for (slot& slot : mSlots)
		{
			unlinkSlot(slot.object);
		}
	}


    void clearAllConnections() override
    {
        std::unique_lock lock(mSlotsLock);
        mSlots.clear();
    }
    void clearAllConnectionsWith(AObject* object) override
    {
        std::unique_lock lock(mSlotsLock);
        for (auto it = mSlots.begin(); it != mSlots.end();)
        {
            if (it->object == object)
            {
                it = mSlots.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
};
#include <AUI/Thread/AThread.h>

template <typename ... Args>
void ASignal<Args...>::invokeSignal()
{
	std::unique_lock lock(mSlotsLock);
	for (auto i = mSlots.begin(); i != mSlots.end();)
	{
		try
		{
			if (i->object->getThread() != AThread::current())
			{
				i->object->getThread()->enqueue([=]() {
					try {
						(std::apply)(i->func, mArgs);
					}
					catch (Disconnect)
					{
						std::unique_lock lock(mSlotsLock);
						unlinkSlot(i->object);
						mSlots.erase(i);
					}
				});
			}
			else
			{
                (std::apply)(i->func, mArgs);
			}
			++i;
		}
		catch (Disconnect)
		{
			unlinkSlot(i->object);
			i = mSlots.erase(i);
		}
	}
}

template<typename... Args>
using emits = ASignal<Args...>;

#define signals public