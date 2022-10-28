/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <tuple>
#include <variant>
#include <cstdint>
#include <iterator>
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
        const T& mIterable;

    public:
        reverse_iterator_wrap(const T& mIterable) : mIterable(mIterable) {}

        auto begin() {
            return mIterable.rbegin();
        }
        auto end() {
            return mIterable.rend();
        }
    };

    template<typename Iterator>
    struct range {
    private:
        Iterator mBegin;
        Iterator mEnd;
    public:
        range(Iterator mBegin, Iterator mEnd) : mBegin(mBegin), mEnd(mEnd) {}

        ~range() {
            AUI_NO_OPTIMIZE_OUT(range::size)
        }

        template<typename Container>
        range(Container& c): mBegin(c.begin()), mEnd(c.end()) {

        }

        [[nodiscard]]
        bool empty() const noexcept {
            return mBegin == mEnd;
        }

        [[nodiscard]]
        std::size_t size() const noexcept {
            return std::distance(mBegin, mEnd);
        }

        [[nodiscard]]
        Iterator& begin() noexcept {
            return mBegin;
        }

        [[nodiscard]]
        Iterator& end() noexcept {
            return mEnd;
        }

        [[nodiscard]]
        Iterator& begin() const noexcept {
            return mBegin;
        }

        [[nodiscard]]
        Iterator& end() const noexcept {
            return mEnd;
        }

        [[nodiscard]]
        const auto& first() const {
            return *mBegin;
        }

        [[nodiscard]]
        const auto& last() const {
            return *std::prev(mEnd);
        }
    };

    // deduction guide
    template<typename Container>
    range(Container& c) -> range<decltype(c.begin())>;

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
     * @code{cpp}
     * std::array<int, 3> ints = { 1, 2, 3 };
     * std::array<std::string, 3> strings = { "one", "two", "three" };
     * for (auto&[i, s] : aui::zip(ints, strings)) {
     *     std::cout << i << ", " << s << std::endl;
     * }
     * @endcode
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
     * @ingroup useful_macros
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
        std::enable_if_t<!impl::is_forward_iterator<Iterator>,
                         decltype((iterator + 1).base())
                         > {
        return (iterator + 1).base();
    }

    template<typename Iterator>
    auto reverse_iterator_direction(Iterator iterator) noexcept ->
        std::enable_if_t<impl::is_forward_iterator<Iterator>,
                         decltype(std::make_reverse_iterator(std::declval<Iterator>()))
                         > {

        return std::make_reverse_iterator(iterator + 1);
    }
}