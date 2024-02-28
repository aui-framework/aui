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

#pragma once
#include <queue>
#include <AUI/Core.h>
#include <AUI/Traits/containers.h>


/**
 * @brief A std::queue with AUI extensions.
 * @ingroup core
 */
template <class StoredType>
class AQueue : public std::queue<StoredType>
{
	using super = std::queue<StoredType>;
public:
    /**
     * Removes all occurrences of <code>item</code>.
     * @param item element to remove.
     */
    void removeAll(const StoredType& item) noexcept
    {
        aui::container::remove_all(*this, item);
    }

    /**
     * Removes first occurrence of <code>item</code>.
     * @param item element to remove.
     */
    void removeFirst(const StoredType& item) noexcept
    {
        aui::container::remove_first(*this, item);
    }


	AQueue<StoredType>& operator<<(const StoredType& rhs)
	{
		super::push(rhs);
		return *this;
	}


	AQueue<StoredType>& operator<<(StoredType&& rhs)
	{
		super::push(std::forward<StoredType>(rhs));
		return *this;
	}

    /**
     * @return true if container contains an element, false otherwise.
     */
    bool contains(const StoredType& value) const noexcept {
        return aui::container::contains(*this, value);
    }


    /**
     * @return true if <code>c</code> container is a subset of this container, false otherwise.
     */
    template<typename OtherContainer>
    bool isSubsetOf(const OtherContainer& c) const noexcept
    {
        return aui::container::is_subset(*this, c);
    }

    /**
     * @brief Pops the element and returns it. If queue is empty, the result of <code>factory()</code> returned.
     * The result of <code>factory()</code> is never added to the queue.
     * @return result of <code>front()</code> if not empty; result of <code>factory()</code> otherwise
     */
    template<typename Factory>
    [[nodiscard]]
    StoredType popOrGenerate(Factory&& factory) noexcept(noexcept(factory())) {
        if (super::empty()) {
            return factory();
        }
        auto v = std::move(super::front());
        super::pop();
        return v;
    }
};
