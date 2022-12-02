// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once


#include <AUI/Thread/AThreadPool.h>
#include <AUI/Traits/iterators.h>

namespace aui {
    /**
     * @brief Distributes iterator range over machine's threads.
     * I.e. range of 32 elements on a 4-core machine would be split by 4 pieces of 8 elements.
     * @tparam Iterator iterator type. Must implement <code>operator- </code>
     * @param begin begin iterator
     * @param end en iterator
     * @param functor callback. <code>ReturnType(Iterator begin, Iterator end);</code>
     * @return AFutureSet<ReturnType>, where ReturnType is a return type of functor
     * @example Typical usage:
     *
     * @code{cpp}
     * AVector<int> container = { ... };
     * aui::parallel(container.begin(),
     *               container.end(),
     *               [](const AVector<int>::iterator& begin,
     *                  const AVector<int>::iterator& end) {
     *     for (auto it = begin; it != end; ++it) {
     *         // *it
     *     }
     * };
     * @endcode
     *
     * @see <code>AUI_PARALLEL_MP</code>
     */
    template<typename Iterator, typename Functor>
    auto parallel(Iterator begin, Iterator end, Functor&& functor) {
        return AThreadPool::global().parallel(begin, end, std::forward<Functor>(functor));
    }

    namespace impl::parallel {
        template<typename Container>
        struct LambdaCapturer {
            using iterator = decltype(std::declval<Container>().begin());
            iterator begin, end;
            LambdaCapturer(Container& c):
                begin(c.begin()),
                end(c.end())
            {

            }

            template<typename Lambda>
            auto operator<<(Lambda&& lambda) {
                return aui::parallel(begin, end, std::forward<Lambda>(lambda));
            }
        };
    }
}

/**
 * @brief Shortcut to aui::parallel.
 * @example Typical usage:
 * @code{cpp}
 * AVector<int> container = { ... };
 * auto tasks = AUI_PARALLEL_MP(container) {
 *     for (auto it = begin; it != end; ++it) {
 *         // *it
 *     }
 * };
 * tasks.wait();
 * @endcode
 *
 * @see aui::parallel
 */
#define AUI_PARALLEL_MP(...) aui::impl::parallel::LambdaCapturer(__VA_ARGS__) \
<< [&](const std::decay_t<decltype(aui::impl::parallel::LambdaCapturer(__VA_ARGS__))::iterator>& begin, \
       const std::decay_t<decltype(aui::impl::parallel::LambdaCapturer(__VA_ARGS__))::iterator>& end)