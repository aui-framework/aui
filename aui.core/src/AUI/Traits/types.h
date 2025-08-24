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

#include <type_traits>

namespace aui {

    /**
     * @brief Determines whether <code>T</code> is complete or not.
     * @ingroup useful_templates
     */
    template<typename T, class = void>
    inline constexpr bool is_complete = false;

    /**
     * @brief Determines whether <code>T</code> is complete or not.
     * @ingroup useful_templates
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
     * @ingroup useful_templates
     * @details
     * Example:
     * ```cpp
     * struct GameObject {
     * public:
     *   void setPos(glm::vec3);
     *   void setPos(glm::vec2);
     * };
     * ...
     * auto setPosVec2 = aui::select_overload<glm::vec2>(&GameObject::setPos);
     * ```
     */
    template<typename... Args>
    constexpr impl::select_overload<Args...> select_overload = {};
}