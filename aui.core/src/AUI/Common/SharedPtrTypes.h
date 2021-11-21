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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <memory>
#include <functional>
#include <optional>

class AObject;


template<typename T>
using _weak = std::weak_ptr<T>;

template<typename T>
class _;

namespace aui {
    struct ptr {
        /**
         * Delegates memory management of the raw pointer <code>T* raw</code> to the shared pointer, which is returned
         * @tparam T any type
         * @param raw raw pointer to manage memory of
         * @return shared pointer
         */
        template<typename T>
        static _<T> manage(T* raw);

        /**
         * Delegates memory management of the raw pointer <code>T* raw</code> to the shared pointer, which is returned
         * @tparam T any type
         * @tparam Deleter object implementing <code>operator()(T*)</code>
         * @param raw raw pointer to manage memory of
         * @param deleter
         * @return shared pointer
         */
        template<typename T, typename Deleter>
        static _<T> manage(T* raw, Deleter deleter);

        /**
         * Creates fake shared pointer to <code>T* raw</code> with empty destructor, which does nothing. It's useful
         * when some function accept shared pointer but you have only raw one.
         * @tparam T any type
         * @param raw raw pointer to manage memory of
         * @return shared pointer
         */
        template<typename T>
        static _<T> fake(T* raw);
    };
}


/**
 * \brief std::shared_ptr<T> wrapper
 * \note  Of course, it is not good tone to define a class with _ type but it significantly increases coding speed.
 *        Instead of writing every time std::shared_ptr you should write only the _ symbol.
 */
template<typename T>
class _ : public std::shared_ptr<T>
{
friend struct aui::ptr;
private:
	using parent = std::shared_ptr<T>;

    _(T* raw, std::nullopt_t): std::shared_ptr<T>(raw) {

    }

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

    using std::shared_ptr<T>::shared_ptr;

    /**
     * <p>Trap constructor</p>
     * <p>In order to make shared pointer from the raw one, please explicitly specify how do you want manage memory by
     * using either <code>aui::ptr::manage</code> or <code>aui::ptr::fake</code>.
     */
    _(T* v) {
        static_assert(false, "use either aui::ptr::manage or aui::ptr::fake");
    }

    /**
     * @return weak reference
     */
    [[nodiscard]]
    _weak<T> weak() {
        return _weak<T>(*this);
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
	const _<T>& operator<<(Arg&& value) const {
        (*parent::get()) << std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
	_<T>& operator<<(Arg&& value) {
        (*parent::get()) << std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
	const _<T>& operator+(Arg&& value) const {
        (*parent::get()) + std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
	_<T>& operator+(Arg&& value) {
        (*parent::get()) + std::forward<Arg>(value);
        return *this;
    }
    template<typename Arg>
	const _<T>& operator-(Arg&& value) const {
        (*parent::get()) - std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
	_<T>& operator-(Arg&& value) {
        (*parent::get()) - std::forward<Arg>(value);
        return *this;
    }
    template<typename Arg>
	_<T>& operator>>(Arg&& value) {
        (*parent::get()) >> std::forward<Arg>(value);
        return *this;
    }

    template<typename...Args>
	_<T>& operator()(const Args&... value) {
        (*parent::get())(value...);
        return *this;
    }
    template<typename...Args>
	auto operator()(Args&&... value) {
        return (*parent::get())(std::forward<Args>(value)...);
    }
};


namespace aui {
    template<typename T>
    _<T> ptr::manage(T* raw) {
        return _<T>(raw, std::nullopt);
    }

    template<typename T>
    _<T> ptr::fake(T* raw) {
        return _<T>(raw, [](T*) {});
    }

    template<typename T, typename Deleter>
    _<T> ptr::manage(T* raw, Deleter deleter) {
        return _<T>(raw, deleter);
    }
}


template<typename T>
using _unique = std::unique_ptr<T>;


template<typename TO, typename FROM>
inline _<TO> _cast(_<FROM> object)
{
	return std::dynamic_pointer_cast<TO, FROM>(object);
}


/**
 * Parody to null-safety.
 * @example
 * <code>
 * _&lt;AView&gt; view;<br/>
 * nullsafe(view)->enable();
 * </code>
 */
#define nullsafe(s) if(s)s