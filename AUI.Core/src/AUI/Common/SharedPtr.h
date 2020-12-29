#pragma once

#include "SharedPtrTypes.h"

#include <AUI/Common/AObject.h>
#include <AUI/Thread/AThread.h>

template<typename T, typename... Args>
inline auto _new(Args&& ... args)
{
	if constexpr (std::is_base_of_v<AObject, T>) {
		auto o = new T(args...);
		return _<T>(o, [](T* obj)
		{
			static_cast<AObject*>(obj)->getThread()->enqueue([obj]()
			{
                obj->clearSignals();
				static_cast<AObject*>(obj)->getThread()->enqueue([obj]()
				{
					delete obj;
				});
			});
		});
	}
	else {
		return _<T>(std::make_shared<T>(std::forward<Args>(args)...));
	}
}


template<typename T>
template<typename SignalField, typename Object, typename Function>
inline _<T>& _<T>::connect(SignalField signalField, Object object, Function function) {
    AObject::connect(parent::get()->*signalField, object, function);
    return *this;
}


struct shared_t {};

#define shared shared_t()

template<typename T>
inline auto operator>>(T& object, shared_t)
{
    return _new<T>(std::move(object));
}
template<typename T>
inline auto operator>>(T&& object, shared_t)
{
    return _new<T>(std::forward<T>(object));
}