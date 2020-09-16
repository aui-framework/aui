#pragma once

#include <type_traits>

namespace aui {
    template<typename boolean, typename OnTrue, typename OnFalse>
    struct ternary {};

    template<typename OnTrue, typename OnFalse>
    struct ternary<std::true_type, OnTrue, OnFalse> {
        using value = OnTrue;
    };

    template<typename OnTrue, typename OnFalse>
    struct ternary<std::false_type, OnTrue, OnFalse> {
        using value = OnFalse;
    };

    template<typename boolean, typename OnTrue, typename OnFalse>
    using ternary_v = typename ternary<boolean, OnTrue, OnFalse>::value;
}