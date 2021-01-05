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
#include "AUI/Common/SharedPtr.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/AException.h"

/**
 * \brief Minimal Dependency Injection implementation
 */
namespace Autumn
{
	namespace detail
	{
		template <typename T>
		_<T>& storage()
		{
			static _<T> t = nullptr;
			return t;
		}
		template <typename T>
		AMap<AString, _<T>>& storageMap()
		{
			static AMap<AString, _<T>> t;
			return t;
		}
	}
	
	template <typename T>
	_<T> get()
	{
		return detail::storage<T>();
	}
	template <typename T>
	void put(_<T> obj)
	{
		detail::storage<T>() = obj;
	}


	template <typename T>
	_<T> get(const AString& name)
	{
		if (auto c = detail::storageMap<T>().contains(name))
		{
			return c->second;
		}
		throw AException("No such object in Autumn: " + name);
	}
	template <typename T>
	void put(const AString& name, _<T> obj)
	{
		detail::storageMap<T>()[name] = obj;
	}



    /**
     * \brief Temporary replace object in Autumn.
     * \tparam object type.
     */
    template <typename T>
    class Temporary {
    private:
        _<T> mPrevValue;
        std::function<void()> mOnDeleteCallback;

    public:
        Temporary(const _<T>& newValue) {
            mPrevValue = Autumn::get<T>();
            Autumn::put(newValue);
        }

        ~Temporary() {
            Autumn::put(mPrevValue);
            if (mOnDeleteCallback)
                mOnDeleteCallback();
        }

        void onDelete(const std::function<void()>& callback) {
            mOnDeleteCallback = callback;
        }
    };

    template <class T>
    struct construct {
        template<typename... Args>
	    static _<T> with() {
            return _new<T>(Autumn::get<Args>()...);
        }
    };
}