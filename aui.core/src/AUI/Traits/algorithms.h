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

//
// Created by Alex2772 on 11/28/2021.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <cmath>
#include "iterators.h"

namespace aui {

    /**
     * @brief The result of the predicate passed to aui::binary_search.
     */
    enum class BinarySearchResult {
        /**
         * @brief Means <code>it</code> is the searched value/
         */
        MATCH,

        /**
         * @brief Means the searched value is to the left of <code>it</code>
         */
         LEFT,

        /**
         * @brief Means the searched value is to the right of <code>it</code>
         */
        RIGHT
    };


    /**
     * @brief aui::binary_search helper to find a value the Predicate returns the nearest to zero result from.
     * @tparam Predicate
     */
    template<typename Predicate, typename Iterator>
    struct BinarySearchNearestToZero {
    public:
        BinarySearchNearestToZero(Predicate predicate, aui::range<Iterator> range): mPredicate(std::move(predicate)), mRange(range) {}

        BinarySearchResult operator()(Iterator c) {
            auto next = std::next(c);
            if (next == mRange.end()) {
                return BinarySearchResult::MATCH;
            }

            auto v1 = mPredicate(c);
            auto v2 = mPredicate(next);

            if (v1 <= 0 && v2 > 0) {
                if (v2 + v1 < 0) {
                    return BinarySearchResult::RIGHT;
                }
                return BinarySearchResult::MATCH;
            }
            if (v1 > 0) {
                if (mRange.begin() == c) {
                    return BinarySearchResult::MATCH;
                }
                if (auto v0 = mPredicate(std::prev(c)); v1 + v0 < 0) {
                    return BinarySearchResult::MATCH;
                }
                return BinarySearchResult::LEFT;
            }

            return BinarySearchResult::RIGHT;
        }
    private:
        Predicate mPredicate;
        aui::range<Iterator> mRange;
    };


    /**
     * @brief Performs binary search on range [begin, end).
     *
     * @details
     * For binary search to succeed, the range [begin, end) must be at least partially ordered with respect to predicate:
     * <ul>
     *   <li>Predicate returns BinarySearchResult::RIGHT for the all elements preceding the find subject.</li>
     *   <li>Predicate returns BinarySearchResult::LEFT for the all elements following the find subject.</li>
     *   <li>Predicate returns BinarySearchResult::MATCH for the find subject.</li>
     * </ul>
     *
     * It like a game "cold-warm" where <code>predicate</code> says searched object is stored either to the left or
     * right relatively to the find subject.
     *
     * @tparam Iterator iterator type
     * @tparam Predicate predicate (comparator) type
     * @param begin range begin
     * @param end range end
     * @param predicate predicate: <code>int predicate(const Iterator& it)</code>
     *        <p>
     *        <code>Predicate</code>:
     *          <dl>
     *              <dt><b>Returns</b></dt>
     *              <dd>
     *                  <code>BinarySearchResult</code>
     *              </dd>
     *              <dt><b>Accepts</b></dt>
     *              <dd>
     *                  An iterataor
     *              </dd>
     *          </dl>
     *        </p>
     * @return either iterator where <code>predicate</code> returned <code>0</code> or <code>end</code> if searched
     *         value could not be found.
     *
     * <dl>
     *     <dt><b>O(n)</b></dt>
     *     <dd>
     *         log<sub>2</sub>(end - begin)
     *     </dd>
     * </dl>
     */
    template<typename Iterator, typename Predicate>
    Iterator binary_search(Iterator begin, Iterator end, Predicate predicate) {
        static_assert(std::is_same_v<BinarySearchResult, decltype(predicate(std::declval<Iterator>()))>, "predicate should return BinarySearchResult");

        size_t rangeEnd = end - begin;
        size_t rangeBegin = 0;


        for (size_t i; rangeBegin < rangeEnd; ) {
            i = (rangeEnd - rangeBegin - 1) / 2 + rangeBegin;
            auto it = begin + i;
            switch (predicate(it)) {
                case BinarySearchResult::MATCH:
                    return it;

                case BinarySearchResult::RIGHT:
                    rangeBegin = i + 1;
                    break;
                case BinarySearchResult::LEFT:
                    rangeEnd = i;
                    break;
            }

        }
        return end;
    }
}