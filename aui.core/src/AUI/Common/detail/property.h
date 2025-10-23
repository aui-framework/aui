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

#include <AUI/Common/ASignal.h>
#include <AUI/Common/APropertyPrecomputed.h>
#include <AUI/Common/PropertyModifier.h>
#include <AUI/Traits/unsafe_declval.h>

namespace aui::detail::property {

template <typename Projection, typename Source>
concept ProjectionBidirectional = requires(Projection&& projectionBidirectional, Source&& source) {
    // projection must accept SOURCE type.
    { projectionBidirectional } -> aui::invocable<const Source&>;

    // projection must be able to accept DESTINATION type to perform the opposite conversion.
    { projectionBidirectional } -> aui::invocable<const std::invoke_result_t<Projection, const Source&>&>;

    // projection's SOURCE type must be distinguishable from DESTINATION type.
    requires not aui::same_as<std::decay_t<decltype(projectionBidirectional(source))>, std::decay_t<Source>>;
};

template <typename Property>                 // can't use AAnyProperty here, as concept would depend on itself
auto makeAssignment(Property&& property) {   // note the rvalue reference template argument here:
    // pass your property as std::move(*this) if your
    // property-compliant struct is temporary! otherwise you'll
    // spend your weekend on debugging segfaults :)
    using Underlying = std::decay_t<decltype(*property)>;
    struct Invocable {
        Property property;
        void operator()(const Underlying& value) const {
            // avoid property assignment loop (bidirectional connection)
            // PropertyCommonTest.Property2PropertyBoth
            if (property.changed.isAtSignalEmissionState()) {
                return;
            }
            const_cast<Property&>(property) = std::move(value);
        };
    } i = { std::forward<Property>(property) };

    return ASlotDef<decltype(property.boundObject()), decltype(i)> {
        property.boundObject(),
        std::move(i),
    };
}
}   // namespace aui::detail::property

#define AUI_DETAIL_BINARY_OP(op)                                                                                      \
    template <AAnyProperty T, typename Rhs>                                                                           \
    inline decltype(auto) operator op(T&& lhs, Rhs&& rhs) { /* property forwarding op */                              \
        static_assert(                                                                                                \
            requires {                                                                                                \
                aui::unsafe_declval<std::decay_t<typename std::decay_t<T>::Underlying>&>() op std::forward<Rhs>(rhs); \
            }, "AProperty: this binary operator is not defined for underlying type.");                                \
        /* try const operator first */                                                                                \
        if constexpr (requires { *lhs op std::forward<Rhs>(rhs); }) {                                                 \
            return *lhs op std::forward<Rhs>(rhs);                                                                    \
        } else {                                                                                                      \
            /* fallback to a non-const version, involving writeScope() */                                             \
            return *lhs.writeScope() op std::forward<Rhs>(rhs);                                                       \
        }                                                                                                             \
    }                                                                                                                 \
// note: sync this PropertyModifier.h

// comparison
AUI_DETAIL_BINARY_OP(==)
AUI_DETAIL_BINARY_OP(!=)
AUI_DETAIL_BINARY_OP(<=)
AUI_DETAIL_BINARY_OP(>=)
AUI_DETAIL_BINARY_OP(<)
AUI_DETAIL_BINARY_OP(>)

// arithmetic/logical
AUI_DETAIL_BINARY_OP(+)
AUI_DETAIL_BINARY_OP(-)
AUI_DETAIL_BINARY_OP(*)
AUI_DETAIL_BINARY_OP(/)
AUI_DETAIL_BINARY_OP(&)
AUI_DETAIL_BINARY_OP(&&)
AUI_DETAIL_BINARY_OP(|)
AUI_DETAIL_BINARY_OP(||)
AUI_DETAIL_BINARY_OP(<<)
AUI_DETAIL_BINARY_OP(>>)

// assignment
AUI_DETAIL_BINARY_OP(+=)
AUI_DETAIL_BINARY_OP(-=)
AUI_DETAIL_BINARY_OP(*=)
AUI_DETAIL_BINARY_OP(/=)
AUI_DETAIL_BINARY_OP(&=)
AUI_DETAIL_BINARY_OP(|=)
AUI_DETAIL_BINARY_OP(<<=)
AUI_DETAIL_BINARY_OP(>>=)

#undef AUI_DETAIL_BINARY_OP

/*
// UNCOMMENT THIS to test biProjected
static_assert(requires (AProperty<int>& intProperty) {
    { intProperty.biProjected(aui::lambda_overloaded {
      [](int) -> AString { return ""; },
      [](const AString&) -> int { return 0; },
    }).value() } -> aui::convertible_to<AString>;

    { intProperty.biProjected(aui::lambda_overloaded {
        [](int) -> AString { return ""; },
        [](const AString&) -> int { return 0; },
    }) = "AString" };


    { intProperty.biProjected(aui::lambda_overloaded {
        [](int) -> AString { return ""; },
        [](const AString&) -> int { return 0; },
    }) };

    { intProperty.biProjected(aui::lambda_overloaded {
        [](int) -> AString { return ""; },
        [](const AString&) -> int { return 0; },
    }).assignment() } -> aui::invocable<AString>;
});
*/

template <APropertyReadable T>
struct fmt::formatter<T> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    auto format(T& c, format_context& ctx) const { return fmt::format_to(ctx.out(), "{}", *c); }
};
