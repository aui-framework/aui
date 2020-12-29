#pragma once


namespace aui::traits {
    template<typename value, typename OnTrue, typename OnFalse>
    struct ternary {};

    template<typename OnTrue, typename OnFalse>
    struct ternary<std::true_type, OnTrue, OnFalse> {
        using value = OnTrue;
    };
    template<typename OnTrue, typename OnFalse>
    struct ternary<std::false_type, OnTrue, OnFalse> {
        using value = OnFalse;
    };

    template<typename value, typename OnTrue, typename OnFalse>
    using ternary_v = typename ternary<value, OnTrue, OnFalse>::value;
}