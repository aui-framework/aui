/**
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

namespace aui {

    /**
     * \brief Helper wrapper for objects implementing rbegin and rend for using in foreach loops
     * \example
     * <pre>
     * for (auto& view : aui::reverse_iterator_wrap(mViews)) { ... }
     * </pre>
     * \tparam T ReverseIterable
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


        template<typename Container>
        range(Container& c): mBegin(c.begin()), mEnd(c.end()) {

        }

        Iterator& begin() {
            return mBegin;
        }

        Iterator& end() {
            return mEnd;
        }
    };

    template<typename...Items>
    struct joined_range {
    private:
        std::tuple<Items*...> mItems;

    public:
        joined_range(const std::tuple<Items*...>& items) : mItems(items) {}

    public:
        using joined_range_t = joined_range<Items...>;

        struct my_iterator {
        friend joined_range_t;
        private:
            joined_range_t& mJoinedRange;
            std::variant<typename Items::iterator...> mIterator;
            std::size_t mIndex = 0;

        public:
            my_iterator(joined_range_t& mJoinedRange) :
                mJoinedRange(mJoinedRange),
                mIterator(std::get<0>(mJoinedRange.mItems)->begin()),
                mIndex(0)
                {}

            my_iterator& operator++()
            {
                std::visit([](auto&& arg){
                    ++arg;
                }, mIterator);
                return *this;
            }

            bool operator!=(const my_iterator& other) const
            {
                return mIterator != other.mIterator;
            }
            bool operator==(const my_iterator& other) const
            {
                return mIterator == other.mIterator;
            }

            auto operator->()
            {
                return std::visit([](auto&& arg){
                    return &*arg;
                }, mIterator);
            }
            auto operator*()
            {
                return *this;
            }

            auto operator*() const
            {
                return *this;
            }
        };

        my_iterator begin() {
            return my_iterator{*this };
        }
        my_iterator end() {
            return my_iterator{*this };
        }
    };

    template<typename... Items>
    joined_range<Items...> join(Items... items) {
        return { (&items)... };
    }
}