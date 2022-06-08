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

namespace aui::container {

    /**
     * Removes element at the specified index.
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd><code>index</code> points to the existing element.</dd>
     * </dl>
     * @param index index of the element.
     */
    template<typename Container>
    void remove_at(Container& c, size_t index) noexcept {
        assert(("index out of bounds" && c.size() > index));
        c.erase(c.begin() + index);
    }

    /**
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
     * @return true if container contains an element, false otherwise.
     */
    template<typename Container>
    [[nodiscard]]
    bool contains(const Container& c, const typename Container::const_reference value) noexcept {
        return std::find(c.begin(), c.end(), value) != c.end();
    }

    /**
     * @return true if container contains an element, false otherwise.
     */
    template<typename Iterator>
    [[nodiscard]]
    bool contains(Iterator begin, Iterator end, const typename std::iterator_traits<Iterator>::value_type& value) noexcept {
        return std::find(begin, end, value) != end;
    }

    /**
     * Removes all occurrences of <code>value</code>.
     */
    template<typename Container>
    void remove_all(Container& container, typename Container::const_reference value) noexcept {
        container.erase(std::remove_if(container.begin(), container.end(), [&](typename Container::const_reference probe)
        {
            return value == probe;
        }), container.end());
    }

    /**
     * Removes first occurrence of <code>value</code>.
     */
    template<typename Container>
    void remove_first(Container& container, typename Container::const_reference value) noexcept {
        auto it = std::find(container.begin(), container.end(), value);
        if (it != container.end()) {
            container.erase(it);
        }
    }

    template<typename Iterator, typename UnaryOperation>
    [[nodiscard]]
    auto to_map(Iterator begin, Iterator end, UnaryOperation&& transformer); // implemented in AMap.h

    /**
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