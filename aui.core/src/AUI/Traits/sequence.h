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

//
// Created by Alex2772 on 11/28/2021.
//

#pragma once

namespace aui {
    template<typename T = int>
    struct sequence_traits {
        template<T first_, T... up_>
        struct of {
            /**
             * Accepts integer sequence in template parameters and returns the value of the first integer.
             * @return value of the first integer.
             */
            static constexpr T first() {
                return first_;
            }

            /**
             * Accepts integer sequence in template parameters and returns the value of the last integer.
             * @return value of the last integer.
             */
            static constexpr T last() {
                constexpr T ints[] = {first_, up_...};
                return ints[sizeof...(up_)];
            }


            /**
             * Accepts integer sequence in template parameters and returns the value of the specified integer by it's index.
             * @param index index of the integer
             * @return value of the specified integer.
             */
            static constexpr T at(unsigned index) {
                constexpr T ints[] = {first_, up_...};
                return ints[index];
            }
        };
    };
}