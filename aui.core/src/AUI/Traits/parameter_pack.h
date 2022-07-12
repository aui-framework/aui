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