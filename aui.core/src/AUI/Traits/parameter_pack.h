// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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

#include <algorithm>

namespace aui {
    namespace impl {
        template<typename First, typename... Up>
        struct first {
            using type = First;
        };
    }

    /**
     * @brief Provides an easy way to iterate over a parameter pack.
     * @ingroup useful_templates
     * @details
     * <table>
     *   <tr>
     *     <td>without</td>
     *     <td>with</td>
     *   </tr>
     *   <tr>
     *     <td>
     *       @code{cpp}
     *       template<typename Arg, typename... Args> void helperFunc(Arg&& arg, Args&&... args) {
     *          // do something with arg
     *          std::cout << std::forward<Arg>(arg) << std::endl;
     *
     *          if constexpr (sizeof...(args) > 0) {
     *              helperFunc(std::forward<Args>(args)...); // continue iteration
     *          }
     *       }
     *       template<typename... Args> void yourFunc(Args&&... args) {
     *           helperFunc(std::forward<Args>(args)...);
     *       }
     *       @endcode
     *     </td>
     *     <td>
     *       @code{cpp}
     *       template<typename... Args> void yourFunc(Args&&... args) {
     *           aui::parameter_pack::for_each([](auto&& i) {
     *               // do something with arg
     *               std::cout << i << " ";
     *           }, std::forward<Args>(args)...);
     *       }
     *       @endcode
     *     </td>
     *   </tr>
     * </table>
     */
    namespace parameter_pack {
        template<typename Callable, typename... Args>
        static void for_each(Callable&& c, Args&&... args) {
            (..., [&] {
                c(std::forward<Args>(args));
            }());
        }
        template<typename... Types>
        using first = typename impl::first<Types...>::type;
    };
}