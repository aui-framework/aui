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