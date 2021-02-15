/**
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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

template<typename T, typename E>
inline auto _new(std::initializer_list<E> il) {

    if constexpr (std::is_base_of_v<AObject, T>) {
        auto o = new T(il.begin(), il.end());
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
        return _<T>(std::make_shared<T>(il.begin(), il.end()));
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