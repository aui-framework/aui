#pragma once

#include "SharedPtrTypes.h"

#include "AUI/Common/AObject.h"
#include "AUI/Thread/AThread.h"

template<typename T, typename... Args>
inline auto _new(Args&& ... args)
{
	if constexpr (std::is_base_of_v<AObject, T>) {
		auto o = new T(args...);
		return _<T>(o, [](T* obj)
		{
			obj->clearSignals();
			static_cast<AObject*>(obj)->getThread()->enqueue([obj]()
			{
				static_cast<AObject*>(obj)->getThread()->enqueue([obj]()
				{
					delete obj;
				});
			});
		});
	}
	else {
		return _<T>(std::make_shared<T>(args...));
	}
}


template<typename T>
template<typename SignalField, typename Object, typename Function>
inline _<T>& _<T>::connect(SignalField signalField, Object object, Function function) {
    AObject::connect(parent::get()->*signalField, object, function);
    return *this;
}