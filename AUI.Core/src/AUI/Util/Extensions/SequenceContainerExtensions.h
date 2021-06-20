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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 07.01.2021.
//


#pragma once

#include "CommonContainerExtensions.h"

template <typename Container>
class SequenceContainerExtensions: public CommonContainerExtensions<Container> {
public:
    using StoredType = typename Container::value_type;
    using Iterator = typename Container::iterator;
    using CommonContainerExtensions<Container>::CommonContainerExtensions;

    inline SequenceContainerExtensions<Container>& operator<<(const StoredType& rhs)
    {
        Container::push_back(rhs);
        return *this;
    }
    inline SequenceContainerExtensions<Container>& operator<<(StoredType&& rhs)
    {
        Container::push_back(std::forward<StoredType>(rhs));
        return *this;
    }
    inline SequenceContainerExtensions<Container>& operator<<(const Container& rhs)
    {
        for (auto& item : rhs)
            Container::push_back(item);
        return *this;
    }


    StoredType& first()
    {
        return Container::front();
    }
    const StoredType& first() const
    {
        return Container::front();
    }

    StoredType& last()
    {
        return Container::back();
    }
    const StoredType& last() const
    {
        return Container::back();
    }

    [[nodiscard]]
    size_t indexOf(const StoredType& t) const
    {
        for (size_t i = 0; i < Container::size(); ++i)
        {
            if ((*this)[i] == t)
                return i;
        }

        return static_cast<size_t>(-1);
    }


    void sort() {
        std::sort(Container::begin(), Container::end());
    }

    template<typename Comparator>
    void sort(Comparator comparator) {
        std::sort(Container::begin(), Container::end(), comparator);
    }

    void removeAt(size_t index)
    {
        this->erase(Container::begin() + index);
    }

};