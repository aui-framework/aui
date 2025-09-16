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

template <typename Property, typename Projection>
auto makeReadonlyProjection(Property&& property, Projection&& projection) {
    using Underlying = std::decay_t<decltype(*property)>;
    auto signalProjected = property.changed.projected(projection);
    using Signal = decltype(signalProjected);
    using ProjectionResult = std::invoke_result_t<Projection, Underlying>;
    struct PropertyReadProjection {
    protected:
        Property wrappedProperty;
        Projection projection;

    public:
        Signal changed;
        PropertyReadProjection(Property wrappedProperty, Projection&& projection, Signal changed)
          : wrappedProperty(wrappedProperty), projection(std::move(projection)), changed(changed) {}
        using Underlying = ProjectionResult;

        [[nodiscard]]
        auto boundObject() const {
            return wrappedProperty.boundObject();
        }

        [[nodiscard]]
        Underlying value() const {
            return std::invoke(projection, wrappedProperty.value());
        }

        [[nodiscard]]
        Underlying operator*() const noexcept {
            return value();
        }

        [[nodiscard]] operator Underlying() const { return value(); }
    };
    static_assert(
        APropertyReadable<PropertyReadProjection>, "PropertyReadProjection must conform with APropertyReadable");
    return PropertyReadProjection(
        std::forward<Property>(property), std::forward<Projection>(projection), std::move(signalProjected));
}

template <typename PropertyReadProjection, typename ProjectionWrite>
struct PropertyReadWriteProjection : PropertyReadProjection {
    ProjectionWrite projectionWrite;
    using Underlying = typename PropertyReadProjection::Underlying;
    explicit PropertyReadWriteProjection(PropertyReadProjection&& read, ProjectionWrite&& projectionWrite)
      : PropertyReadProjection(std::move(read)), projectionWrite(std::move(projectionWrite)) {}

    template <aui::convertible_to<Underlying> U>
    PropertyReadWriteProjection& operator=(U&& value) noexcept {
        this->wrappedProperty = std::invoke(projectionWrite, std::forward<U>(value));
        return *this;
    }

    /**
     * @brief Notify observers that a change was occurred (no preconditions).
     */
    void notify() { this->wrappedProperty.notify(); }

private:
    friend class ::AObject;

    /**
     * @brief Makes a callable that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(std::move(*this));
    }
};

template <typename Property, aui::not_overloaded_lambda ProjectionRead, aui::not_overloaded_lambda ProjectionWrite>
auto makeBidirectionalProjection(
    Property&& property, ProjectionRead&& projectionRead, ProjectionWrite&& projectionWrite) {
    auto readProjected =
        makeReadonlyProjection(std::forward<Property>(property), std::forward<ProjectionRead>(projectionRead));
    using PropertyReadProjection = decltype(readProjected);
    PropertyReadWriteProjection result(std::move(readProjected), std::forward<ProjectionWrite>(projectionWrite));
    //    static_assert(APropertyWritable<decltype(result)>, "PropertyReadWriteProjection must conform with
    //    APropertyWriteable");
    return result;
}

template <typename Property, ProjectionBidirectional<typename std::decay_t<Property>::Underlying> Projection>
auto makeBidirectionalProjection(Property&& property, Projection&& projection) {
    // we must define non-overloaded lambdas for makeBidirectionalProjection overload.
    using Source = std::decay_t<typename std::decay_t<Property>::Underlying>;
    using Destination = std::decay_t<std::invoke_result_t<Projection, const Source&>>;
    return makeBidirectionalProjection(
        std::forward<Property>(property),
        [projection](const Source& s) -> Destination { return std::invoke(projection, s); },
        [projection](const Destination& d) -> Source { return std::invoke(projection, d); });
}
}   // namespace aui::detail::property

#define AUI_DETAIL_BINARY_OP(op)                                                                                     \
    template <AAnyProperty T, typename Rhs>                                                                          \
    inline decltype(auto) operator op(T&& lhs, Rhs&& rhs) { /* property forwarding op */                             \
        static_assert(                                                                                               \
            requires {                                                                                               \
                aui::unsafe_declval<std::decay_t<typename std::decay_t<T>::Underlying>>() op std::forward<Rhs>(rhs); \
            }, "AProperty: this binary operator is not defined for underlying type.");                               \
        /* try const operator first */                                                                               \
        if constexpr (requires { *lhs op std::forward<Rhs>(rhs); }) {                                                \
            return *lhs op std::forward<Rhs>(rhs);                                                                   \
        } else {                                                                                                     \
            /* fallback to a non-const version, involving writeScope() */                                            \
            return *lhs.writeScope() op std::forward<Rhs>(rhs);                                                      \
        }                                                                                                            \
    }                                                                                                                \
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
