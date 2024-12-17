/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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