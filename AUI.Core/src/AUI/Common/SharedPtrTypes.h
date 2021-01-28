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

#include <memory>
#include <functional>

class AObject;


/**
 * \brief std::shared_ptr<T> wrapper
 *        Of course, it is not good tone to define a class with _ type but it significantly increases coding speed.
 *        Instead of writing every time std::shared_ptr you should write only the _ symbol.
 */
template<typename T>
class _ : public std::shared_ptr<T>
{
private:
	using parent = std::shared_ptr<T>;

public:
    using stored_t = T;

	class SafeCallWrapper
	{
	private:
		_<T>& mPtr;

	public:
		SafeCallWrapper(_<T>& ptr)
			: mPtr(ptr)
		{
		}

		template<typename MemberFunction, typename... Args>
		SafeCallWrapper& operator()(MemberFunction memberFunction, Args&& ... args) {
			if (mPtr)
				(mPtr.get()->*memberFunction)(std::forward<Args>(args)...);
			return *this;
		}
	};

	_() = default;

	template<typename X>
	_(X&& x)
		: parent(std::forward<X>(x))
	{
	}
	template<typename X, typename Y>
	_(X&& x, Y&& y)
		: parent(std::forward<X>(x), std::forward<Y>(y))
	{
	}


	template<typename MemberFunction, typename... Args>
	inline _<T>& operator()(MemberFunction memberFunction, Args&&... args) {
		(parent::get()->*memberFunction)(std::forward<Args>(args)...);
		return *this;
	}

	template<typename SignalField, typename Object, typename Function>
	inline _<T>& connect(SignalField signalField, Object object, Function function);


	template <typename Functor>
	inline _<T>& operator^(Functor functor) {
	    functor(*this);
	    return *this;
	}

	/**
	 * \brief Guarantees that further builder calls will be executed if and only if this pointer
     *        not equal to null.
	 * \return safe builder
	 */
    inline auto safe()
	{
		return SafeCallWrapper(*this);
	}

	// ranged-for loops
	auto begin() {
	    return parent::operator->()->begin();
	}
	auto end() {
	    return parent::operator->()->end();
	}

	// operators

    template<typename Arg>
	_<T>& operator<<(Arg&& value) {
        (*parent::get()) << std::forward<Arg>(value);
        return *this;
    }
    template<typename Arg>
	_<T>& operator>>(Arg&& value) {
        (*parent::get()) >> std::forward<Arg>(value);
        return *this;
    }
};

template<typename T>
using _weak = std::weak_ptr<T>;

template<typename T>
using _unique = std::unique_ptr<T>;


template<typename TO, typename FROM>
inline _<TO> _cast(_<FROM> object)
{
	return std::dynamic_pointer_cast<TO, FROM>(object);
}