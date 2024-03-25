// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

//
// Created by Alex2772 on 11/28/2021.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <AUI/Common/AOptional.h>
#include <AUI/Traits/iterators.h>


namespace aui::container {

    namespace vector_impl { // basic vector implementation

        /**
         * @brief Corrects size so it's aligned to power of 2.
         */
        static constexpr std::size_t ceilPower2(std::size_t size) {
            switch (size) {
                case 0: return 0;
                case 1: return 1;
                default: break;
            }

            size -= 1;
            std::size_t power = 1;
            while (size != 0) {
                size >>= 1;

                if (size == 0) {
                    break;
                }
                ++power;
            }
            return std::size_t(1) << power;
        }

        static_assert(ceilPower2(228) == 256, "check ceilPower2");
        static_assert(ceilPower2(256) == 256, "check ceilPower2");
        static_assert(ceilPower2(512) == 512, "check ceilPower2");
        static_assert(ceilPower2(0) == 0, "check ceilPower2");
        static_assert(ceilPower2(1) == 1, "check ceilPower2");
        static_assert(ceilPower2(2) == 2, "check ceilPower2");

        template<typename T, typename OtherIterator>
        auto insert_no_growth(T*& vectorEnd, T* at, OtherIterator begin, OtherIterator end) {
            auto distance = std::distance(begin, end);

            aui::range insertTargetRange(at, at + distance);

            // shift elements to the right
            aui::range shiftRange(std::prev(insertTargetRange.end()), vectorEnd);
            if (!shiftRange.empty()) {
                auto shiftFrom = std::prev(shiftRange.end());
                auto shiftTo = shiftFrom + distance;
                for (; shiftFrom >= shiftRange.begin(); --shiftFrom, --shiftTo) {
                    if (shiftTo >= vectorEnd) {
                        new (shiftTo) T(std::move(*shiftFrom));
                    } else {
                        *shiftTo = std::move(*shiftFrom);
                    }
                }
            }

            auto it = at;
            for (; it < vectorEnd && begin != end; ++it, ++begin) {
                *it = *begin;
            }

            for (; begin != end; ++it, ++begin) {
                new (it) T(*begin);
            }

            vectorEnd += distance;
            return at;
        }

        template<typename T, typename OtherIterator>
        auto erase(T* vectorBegin, T*& vectorEnd, OtherIterator begin, OtherIterator end) {
            if (begin == end) {
                return std::prev(vectorEnd);
            }

            auto eraseIt = begin;
            auto remainingValueIt = end;

            // move values to the left of erase range
            for (; remainingValueIt != vectorEnd; ++eraseIt, ++remainingValueIt) {
                *eraseIt = std::move(*remainingValueIt);
            }
            // destruct remaining values
            std::size_t destructedValuesCounter = 0;
            for (; eraseIt != vectorEnd; ++eraseIt, ++destructedValuesCounter) {
                eraseIt->~T();
            }
            vectorEnd -= destructedValuesCounter;
            return vectorBegin;
        }
    }

    /**
     * @brief Removes element at the specified index.
     * @ingroup core
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd><code>index</code> points to the existing element.</dd>
     * </dl>
     * @param index index of the element.
     */
    template<typename Container>
    void remove_at(Container& c, size_t index) noexcept {
        AUI_ASSERTX(c.size() > index, "index out of bounds");
        c.erase(c.begin() + index);
    }

    /**
     * @brief Finds the index of the first occurrence of the value.
     * @ingroup core
     * @param value element to find.
     * @return index of the specified element. If element is not found, -1 is returned.
     */
    template<typename Container>
    [[nodiscard]]
    size_t index_of(const Container& c, const typename Container::const_reference value) noexcept {
        auto it = std::find(c.begin(), c.end(), value);
        if (it == c.end()) return static_cast<size_t>(-1);
        return it - c.begin();
    }

    /**
     * @ingroup core
     * @return true if container contains an element, false otherwise.
     */
    template<typename Container>
    [[nodiscard]]
    bool contains(const Container& c, const typename Container::const_reference value) noexcept {
        return std::find(c.begin(), c.end(), value) != c.end();
    }

    /**
     * @ingroup core
     * @return true if container contains an element, false otherwise.
     */
    template<typename Iterator>
    [[nodiscard]]
    bool contains(Iterator begin, Iterator end, const typename std::iterator_traits<Iterator>::value_type& value) noexcept {
        return std::find(begin, end, value) != end;
    }

    /**
     * @brief Removes all occurrences of <code>value</code>.
     * @ingroup core
     */
    template<typename Container>
    void remove_all(Container& container, typename Container::const_reference value) noexcept {
        container.erase(std::remove_if(container.begin(), container.end(), [&](typename Container::const_reference probe)
        {
            return value == probe;
        }), container.end());
    }


    /**
     * Removes all occurrences of <code>item</code> with specified projection.
     * @param item element to remove.
     * @param projection callable that transforms <code>const StoredType&</code> to <code>const T&</code>. Can be any
     *        operator() cappable object, including lambda and pointer-to-member.
     */
    template<typename Container, typename T, typename Projection>
    void remove_all(Container& container, const T& value, const Projection& projection) noexcept {
        container.erase(std::remove_if(container.begin(), container.end(), [&](typename Container::const_reference probe)
        {
            return value == std::invoke(projection, probe);
        }), container.end());
    }

    /**
     * @brief Removes first occurrence of <code>value</code>.
     * @ingroup core
     * @return If the item is removed, it's index returned.
     */
    template<typename Container>
    AOptional<std::size_t> remove_first(Container& container, typename Container::const_reference value) noexcept {
        for (auto it = container.begin(); it != container.end(); ++it) {
            if (*it == value) {
                auto index = std::distance(container.begin(), it);
                container.erase(it);
                return index;
            }
        }
        return {};
    }

    /**
     * @brief Transforms sequence to map.
     * @ingroup core
     */
    template<typename Iterator, typename UnaryOperation>
    [[nodiscard]]
    auto to_map(Iterator begin, Iterator end, UnaryOperation&& transformer); // implemented in AMap.h

    /**
     * @brief Transforms sequence to unordered_map.
     * @ingroup core
     */
    template<typename Iterator, typename UnaryOperation>
    [[nodiscard]]
    auto to_unordered_map(Iterator begin, Iterator end, UnaryOperation&& transformer); // implemented in AUnorderedMap.h

    /**
     * @ingroup core
     * @return true if <code>r</code> container is a subset of <code>l</code> container, false otherwise.
     */
    template<typename LContainer, typename RContainer>
    [[nodiscard]]
    bool is_subset(LContainer& l, RContainer& r) noexcept {
        for (auto& i : r)
        {
            if (!l.contains(i))
            {
                return false;
            }
        }
        return true;
    }
}