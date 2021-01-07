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


template <typename Container>
class CommonContainerExtensions: public Container {
public:
    using StoredType = typename Container::value_type;
    using Iterator = typename Container::iterator;

    using Container::Container;


    template<typename OtherContainer>
    Iterator insertAll(Iterator position, const OtherContainer& c) {
        return Container::insert(position, c.begin(), c.end());
    }


    void remove(const StoredType& item)
    {
        Container::erase(std::remove_if(Container::begin(), Container::end(), [&](const StoredType& probe)
        {
            return item == probe;
        }), Container::end());
    }
    

    template<typename OtherContainer>
    bool isSubsetOf(const OtherContainer& c) const
    {
        for (auto& i : c)
        {
            if (!contains(i))
            {
                return false;
            }
        }
        return true;
    }
    
    bool contains(const StoredType& value) const {
        return std::find(Container::begin(), Container::end(), value) != Container::end();
    }
    
    template<typename Func, typename... Args>
    void forEach(Func f, Args&&... args)
    {
        for (auto& i: *this)
        {
            (i.get()->*f)(std::forward<Args>(args)...);
        }
    }
};