#pragma once

#include "SharedPtrTypes.h"
#include "AUI/Core.h"
#include "AUI/Common/ASet.h"

class AString;
class AAbstractSignal;
class AAbstractThread;

class API_AUI_CORE AObject
{
	friend class AAbstractSignal;
private:
	_<AAbstractThread> mAttachedThread;
	ASet<AAbstractSignal*> mSignals;
	bool mSignalsEnabled = true;
	
protected:
	void disconnect();

public:
	AObject();
	virtual ~AObject();

	void clearSignals();

	template<class Signal, class Object, typename Function>
	static void connect(Signal& signal, Object object, Function function)
	{
		static_assert(std::is_base_of_v<AObject, typename std::remove_pointer<Object>::type>, "the passed object should be a base of the AObject class (use class YourObject: public AObject)");
		static_assert(std::is_pointer_v<Object>, "the object should be a pointer (use &yourObject)");

		if constexpr (std::is_base_of_v<AObject, typename std::remove_pointer<Object>::type> && std::is_pointer_v<Object>) {
			signal.connect(object, function);
		}
	}
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