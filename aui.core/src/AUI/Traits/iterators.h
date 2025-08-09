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

#include <tuple>
#include <variant>
#include <cstdint>
#include <iterator>
#include <ostream>
#include "parameter_pack.h"
#include "macros.h"


namespace aui {

    /**
     * @brief Helper wrapper for objects implementing rbegin and rend for using in foreach loops
     * \example
     * <pre>
     * for (auto& view : aui::reverse_iterator_wrap(mViews)) { ... }
     * </pre>
     * @tparam T ReverseIterable
     */
    template <typename T>
    struct reverse_iterator_wrap {
    private:
        const T* mIterable;

    public:
        explicit reverse_iterator_wrap(const T& mIterable) : mIterable(&mIterable) {}

        auto begin() {
            return mIterable->rbegin();
        }
        auto end() {
            return mIterable->rend();
        }
    };

    template<typename Iterator>
    struct range {
    private:
        Iterator mBegin;
        Iterator mEnd;
    public:
        constexpr range(Iterator mBegin, Iterator mEnd) : mBegin(mBegin), mEnd(mEnd) {}

        constexpr ~range() {
            if constexpr (requires { std::distance(mBegin, mEnd); }) {
                AUI_NO_OPTIMIZE_OUT(range::size)
            }
        }

        template<typename Container>
        constexpr range(Container& c): mBegin(c.begin()), mEnd(c.end()) {

        }


        template<typename Container>
        constexpr range(const Container& c): mBegin(c.begin()), mEnd(c.end()) {

        }

        [[nodiscard]]
        constexpr bool empty() const noexcept {
            return mBegin == mEnd;
        }

        [[nodiscard]]
        constexpr std::size_t size() const noexcept requires requires { std::distance(mBegin, mEnd); }  {
            return std::distance(mBegin, mEnd);
        }

        [[nodiscard]]
        constexpr Iterator& begin() noexcept {
            return mBegin;
        }

        [[nodiscard]]
        constexpr Iterator& end() noexcept {
            return mEnd;
        }

        [[nodiscard]]
        constexpr Iterator begin() const noexcept {
            return mBegin;
        }

        [[nodiscard]]
        constexpr Iterator end() const noexcept {
            return mEnd;
        }

        [[nodiscard]]
        constexpr const auto& first() const {
            return *mBegin;
        }

        [[nodiscard]]
        constexpr const auto& last() const {
            return *std::prev(mEnd);
        }

        [[nodiscard]]
        constexpr bool operator==(const range& rhs) const noexcept;

        [[nodiscard]]
        constexpr bool operator!=(const range& rhs) const noexcept {
            return !(*this == rhs);
        }
    };

    template<typename Container>
    range(Container& c) -> range<decltype(c.begin())>;

    template<typename Container>
    range(const Container& c) -> range<decltype(c.begin())>;

    /**
     * @brief If Container is const, Container::const_iterator is aliased; Container::iterator otherwise.
     */
    template<typename Container>
    using const_iterator_if_const = std::conditional_t<std::is_const_v<Container>,
                                                       typename Container::const_iterator,
                                                       typename Container::iterator>;


    /**
     * @brief Iterates multiple containers in parallel.
     * @ingroup useful_templates
     * @details
     * ```cpp
     * std::array<int, 3> ints = { 1, 2, 3 };
     * std::array<std::string, 3> strings = { "one", "two", "three" };
     * for (auto&[i, s] : aui::zip(ints, strings)) {
     *     std::cout << i << ", " << s << std::endl;
     * }
     * ```
     * <pre>
     * 1, one<br />
     * 2, two<br />
     * 3, three<br />
     * </pre>
     */
    template<typename... Containers>
    struct zip {
    private:
        using iterator_parallel = std::tuple<decltype(std::declval<Containers>().begin())...>;

        iterator_parallel begins_;
        iterator_parallel ends_;

    public:
        struct iterator {
            iterator_parallel iterators_;

            iterator(iterator_parallel iterators) : iterators_(std::move(iterators)) {}

            iterator& operator++() noexcept {
                std::apply([](auto&&... v) {
                    (++v, ...);
                }, iterators_);
                return *this;
            }

            std::tuple<decltype(*std::declval<Containers>().begin())&...> operator*() noexcept {
                return std::apply([](auto&&... v) {
                    return std::tuple<decltype(*std::declval<Containers>().begin())&...>((*v)...);
                }, iterators_);
            }

            bool operator==(const iterator& rhs) const noexcept {
                return iterators_ == rhs.iterators_;
            }

            bool operator!=(const iterator& rhs) const noexcept {
                return iterators_ != rhs.iterators_;
            }
        };

        zip(Containers&... c): begins_(c.begin()...), ends_(c.end()...) {

        }

        iterator begin() noexcept {
            return iterator(begins_);
        }
        iterator end() noexcept {
            return iterator(ends_);
        }
    };

    namespace impl {
        template<typename Iterator, class = void>
        static constexpr bool is_forward_iterator = true;

        template<typename Iterator>
        static constexpr bool is_forward_iterator<std::reverse_iterator<Iterator>> = false;
    }

    /**
     * @brief Reverses iterator direction (i.e. converts iterator to reverse_iterator, reverse_iterator to iterators).
     * @tparam Iterator iterator
     * @param iterator iterator
     * @return same iterator but reverse direction
     *
     * @details
     * A reversed iterator points to the same element of the container.
     *
     * Works on AVector.
     */
    template<typename Iterator>
    auto reverse_iterator_direction(Iterator iterator) noexcept ->
        decltype((iterator + 1).base()) requires (!impl::is_forward_iterator<Iterator>) {
        return (iterator + 1).base();
    }

    template<typename Iterator>
    auto reverse_iterator_direction(Iterator iterator) noexcept ->
        decltype(std::make_reverse_iterator(std::declval<Iterator>())) requires (impl::is_forward_iterator<Iterator>) {

        return std::make_reverse_iterator(iterator + 1);
    }
}

template<typename Iterator>
inline constexpr bool aui::range<Iterator>::operator==(const range& rhs) const noexcept {
    if (size() != rhs.size()) {
        return false;
    }
    for (const auto&[l, r] : aui::zip(*this, rhs)) {
        if (l != r) {
            return false;
        }
    }
    return true;
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, aui::range<T> range) {
    os << "[ ";
    bool isFirst = true;
    for (const auto& v : range) {
        if (isFirst) {
            isFirst = false;
        } else {
            os << ", ";
        }
        os << v;
    }
    return os << " ]";
}
