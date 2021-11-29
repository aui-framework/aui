//
// Created by Alex2772 on 11/28/2021.
//

#pragma once

#include <cstdint>
#include <cstddef>

namespace aui {

    /**
     * @brief Performs binary search on range [begin, end).
     *
     * <p>
     * It like a game "cold-warm" where <code>predicate</code> says searched object is stored either to the left or
     * right relatively to the specified object.
     * </p>
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
     *                  <code>int</code>, where:
     *                  <ul>
     *                      <li><code>0</code> equals <code>it</code> is the searched value</li>
     *                      <li><code>-1</code> equals the searched value is to the left of <code>it</code></li>
     *                      <li><code>1</code> equals the searched value is to the right of <code>it</code></li>
     *                  </ul>
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
        size_t rangeEnd = end - begin;
        size_t rangeBegin = 0;


        for (size_t i; rangeBegin < rangeEnd; ) {
            i = (rangeEnd - rangeBegin - 1) / 2 + rangeBegin;
            auto it = begin + i;
            switch (predicate(it)) {
                case 0:
                    return it;

                case 1:
                    rangeBegin = i + 1;
                    break;
                case -1:
                    rangeEnd = i;
                    break;
            }

        }
        return end;
    }
}