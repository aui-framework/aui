// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#include <type_traits>

namespace aui {

    /**
     * @brief Determines whether <code>T</code> is complete or not.
     * @ingroup useful_traits
     */
    template<typename T, class = void>
    inline constexpr bool is_complete = false;

    /**
     * @brief Determines whether <code>T</code> is complete or not.
     * @ingroup useful_traits
     */
    template<typename T>
    inline constexpr bool is_complete<T, decltype(void(sizeof(T)))> = true;



    namespace impl {
        template<typename... Args>
        struct select_overload {
            template<typename R, typename C>
            constexpr auto operator()(R(C::*ptr)(Args...)) const noexcept -> decltype(ptr) {
                return ptr;
            }
        };
    }
    /**
     * @brief Chooses specific overload of a method.
     * @ingroup useful_traits
     * @details
     * Example:
     * @code{cpp}
     * struct GameObject {
     * public:
     *   void setPos(glm::vec3);
     *   void setPos(glm::vec2);
     * };
     * ...
     * auto setPosVec2 = aui::select_overload<glm::vec2>(&GameObject::setPos);
     * @endcode
     */
    template<typename... Args>
    constexpr impl::select_overload<Args...> select_overload = {};
}