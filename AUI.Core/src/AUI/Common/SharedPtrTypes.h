#pragma once

#include <memory>

class AObject;


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

	template<typename Functor>
	inline _<T>& apply() {
        (*reinterpret_cast<Functor*>(parent::get()))();
	    return *this;
	}

	/**
	 * \brief Гарантирует, что дальнейшие builder-вызовы будут выполняться тогда и только тогда, когда этот указатель
	 *        не равен null.
	 * \return безопасный builder
	 */
    inline auto safe()
	{
		return SafeCallWrapper(*this);
	}

	// для ranged-for циклов
	auto begin() {
	    return parent::operator->()->begin();
	}
	auto end() {
	    return parent::operator->()->end();
	}
};

template<typename T>
using _weak = std::weak_ptr<T>;


template<typename TO, typename FROM>
inline _<TO> _cast(_<FROM> object)
{
	return std::dynamic_pointer_cast<TO, FROM>(object);
}