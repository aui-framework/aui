/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
     * @details
     * Typical usage:
     *
     * ```cpp
     * AVector<int> container = { ... };
     * aui::parallel(container.begin(),
     *               container.end(),
     *               [](const AVector<int>::iterator& begin,
     *                  const AVector<int>::iterator& end) {
     *     for (auto it = begin; it != end; ++it) {
     *         // *it
     *     }
     * };
     * ```
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
 * @details
 * Typical usage:
 * ```cpp
 * AVector<int> container = { ... };
 * auto tasks = AUI_PARALLEL_MP(container) {
 *     for (auto it = begin; it != end; ++it) {
 *         // *it
 *     }
 * };
 * tasks.wait();
 * ```
 *
 * @see aui::parallel
 */
#define AUI_PARALLEL_MP(...) aui::impl::parallel::LambdaCapturer(__VA_ARGS__) \
<< [&](const std::decay_t<decltype(aui::impl::parallel::LambdaCapturer(__VA_ARGS__))::iterator>& begin, \
       const std::decay_t<decltype(aui::impl::parallel::LambdaCapturer(__VA_ARGS__))::iterator>& end)