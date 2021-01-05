/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
#include <tuple>

namespace tuples
{
	namespace detail
	{
		namespace functor
		{
			struct mul {
				template<typename A, typename B>
				auto operator()(A a, B b)
				{
					return a * b;
				}
			};
			struct add {
				template<typename A, typename B>
				auto operator()(A a, B b)
				{
					return a + b;
				}
			};
			struct sub {
				template<typename A, typename B>
				auto operator()(A a, B b)
				{
					return a - b;
				}
			};
			struct div {
				template<typename A, typename B>
				auto operator()(A a, B b)
				{
					return a / b;
				}
			};
		}

		template<typename Function, typename Tuple, size_t... I>
		void call(Function f, Tuple t, std::index_sequence<I...>)
		{
			f(std::get<I>(t)...);
		}

		template<typename Functor, typename Num, typename Tuple, size_t... I>
		Tuple do_transform(Functor f, const Tuple& t, Num n, std::index_sequence<I...>)
		{
			return std::make_tuple(f(std::get<I>(t), n)...);
		}
		template<typename Functor, typename Tuple, size_t... I>
		Tuple do_transform(Functor f, const Tuple& t1, const Tuple& t2, std::index_sequence<I...>)
		{
			return std::make_tuple(f(std::get<I>(t1), std::get<I>(t2))...);
		}
	}

	template<typename Function, typename Tuple>
	void call(Function f, Tuple t)
	{
		detail::call(f, t, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
	}
}

template<typename Num, typename... Types>
std::tuple<Types...> operator+(const std::tuple<Types...>& l, Num n)
{
	return tuples::detail::do_transform(tuples::detail::functor::add(), l, n,
		std::make_index_sequence<std::tuple_size_v<std::tuple<Types...>>>{});
}
template<typename Num, typename... Types>
std::tuple<Types...> operator+(const std::tuple<Types...>& l, const std::tuple<Types...>& r)
{
	return tuples::detail::do_transform(tuples::detail::functor::add(), l, r,
		std::make_index_sequence<std::tuple_size_v<std::tuple<Types...>>>{});
}

template<typename Num, typename... Types>
std::tuple<Types...> operator-(const std::tuple<Types...>& l, Num n)
{
	return tuples::detail::do_transform(tuples::detail::functor::sub(), l, n,
		std::make_index_sequence<std::tuple_size_v<std::tuple<Types...>>>{});
}
template<typename Num, typename... Types>
std::tuple<Types...> operator-(const std::tuple<Types...>& l, const std::tuple<Types...>& r)
{
	return tuples::detail::do_transform(tuples::detail::functor::sub(), l, r,
		std::make_index_sequence<std::tuple_size_v<std::tuple<Types...>>>{});
}


template<typename Num, typename... Types>
std::tuple<Types...> operator/(const std::tuple<Types...>& l, Num n)
{
	return tuples::detail::do_transform(tuples::detail::functor::div(), l, n,
		std::make_index_sequence<std::tuple_size_v<std::tuple<Types...>>>{});
}
template<typename Num, typename... Types>
std::tuple<Types...> operator/(const std::tuple<Types...>& l, const std::tuple<Types...>& r)
{
	return tuples::detail::do_transform(tuples::detail::functor::div(), l, r,
		std::make_index_sequence<std::tuple_size_v<std::tuple<Types...>>>{});
}

template<typename Num, typename... Types>
std::tuple<Types...> operator*(const std::tuple<Types...>& l, Num n)
{
	return tuples::detail::do_transform(tuples::detail::functor::mul(), l, n,
		std::make_index_sequence<std::tuple_size_v<std::tuple<Types...>>>{});
}
template<typename Num, typename... Types>
std::tuple<Types...> operator*(const std::tuple<Types...>& l, const std::tuple<Types...>& r)
{
	return tuples::detail::do_transform(tuples::detail::functor::mul(), l, r,
		std::make_index_sequence<std::tuple_size_v<std::tuple<Types...>>>{});
}