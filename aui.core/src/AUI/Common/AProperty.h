/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Common/ASignal.h>

namespace aui::detail::property {

template<typename Projection, typename Source>
concept ProjectionBidirectional = requires (Projection&& projectionBidirectional, Source&& source) {
    // projection must accept SOURCE type.
    { projectionBidirectional } -> aui::invocable<const Source&>;

    // projection must be able to accept DESTINATION type to perform the opposite conversion.
    { projectionBidirectional } -> aui::invocable<const std::invoke_result_t<Projection, const Source&>&>;

    // projection's SOURCE type must be distinguishable from DESTINATION type.
    requires not aui::same_as<std::decay_t<decltype(projectionBidirectional(source))>, std::decay_t<Source>>;
};

template <typename Property>   // can't use AAnyProperty here, as concept would depend on itself
auto makeAssignment(Property&& property) { // note the rvalue reference template argument here:
                                                         // pass your property as std::move(*this) if your
                                                         // property-compliant struct is temporary! otherwise you'll
                                                         // spend your weekend on debugging segfaults :)
    using Underlying = std::decay_t<decltype(*property)>;
    struct Invocable {
        Property property;
        void operator()(const Underlying& value) const {
            const_cast<Property&>(property) = std::move(value);
        };
    } i = { std::forward<Property>(property) };

    return ASlotDef<decltype(property.boundObject()), decltype(i)> {
        .boundObject = property.boundObject(),
        .invocable = std::move(i),
    };
}

template<typename Property, typename Projection>
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
    static_assert(APropertyReadable<PropertyReadProjection>, "PropertyReadProjection must conform with APropertyReadable");
    return PropertyReadProjection(std::forward<Property>(property), std::forward<Projection>(projection), std::move(signalProjected));
}

template<typename PropertyReadProjection, typename ProjectionWrite>
struct PropertyReadWriteProjection: PropertyReadProjection {
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
     * @brief Makes a callable that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(std::move(*this));
    }
};

template<typename Property, aui::not_overloaded_lambda ProjectionRead, aui::not_overloaded_lambda ProjectionWrite>
auto makeBidirectionalProjection(Property&& property, ProjectionRead&& projectionRead, ProjectionWrite&& projectionWrite) {
    auto readonlyProjection = makeReadonlyProjection(property, std::forward<ProjectionRead>(projectionRead));
    using PropertyReadProjection = decltype(readonlyProjection);
    PropertyReadWriteProjection result(std::move(readonlyProjection), std::forward<ProjectionWrite>(projectionWrite));
//    static_assert(APropertyWritable<decltype(result)>, "PropertyReadWriteProjection must conform with APropertyWriteable");
    return result;
}

template<typename Property, ProjectionBidirectional<typename std::decay_t<Property>::Underlying> Projection>
auto makeBidirectionalProjection(Property&& property, Projection&& projection) {
    // we must define non-overloaded lambdas for makeBidirectionalProjection overload.
    using Source = std::decay_t<typename std::decay_t<Property>::Underlying>;
    using Destination = std::decay_t<std::invoke_result_t<Projection, const Source&>>;
    return makeBidirectionalProjection(
        std::forward<Property>(property),
        [projection](const Source& s) -> Destination { return std::invoke(projection, s); },
        [projection](const Destination& d) -> Source { return std::invoke(projection, d); });
}
}

/**
 * @brief TODO
 * @ingroup property_system
 * @details
 */
template <typename T>
struct AProperty: AObjectBase {
    using Underlying = T;

    T raw;
    emits<T> changed;

    AProperty()
        requires aui::default_initializable<T>
    = default;

    template <aui::convertible_to<T> U>
    AProperty(U&& value) noexcept : raw(std::forward<U>(value)) {}

    AObjectBase* boundObject() {
        return this;
    }

    template <aui::convertible_to<T> U>
    AProperty& operator=(U&& value) noexcept {
        static constexpr auto IS_COMPARABLE = requires { this->raw == value; };
        if constexpr (IS_COMPARABLE) {
            if (this->raw == value) [[unlikely]] {
                return *this;
            }
        }
        this->raw = std::forward<U>(value);
        emit changed(this->raw);
        return *this;
    }

    template <ASignalInvokable SignalInvokable>
    void operator^(SignalInvokable&& t) {
        t.invokeSignal(nullptr);
    }

    [[nodiscard]] operator const T&() const noexcept { return raw; }

    [[nodiscard]]
    const T* operator->() const noexcept {
        return &raw;
    }

    [[nodiscard]]
    const T& operator*() const noexcept {
        return raw;
    }

    [[nodiscard]]
    const T& value() const noexcept {
        return raw;
    }

    [[nodiscard]]
    T& operator*() noexcept {
        return raw;
    }

    /**
     * @brief Makes a callable that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(*this);
    }

    /**
     * @brief Makes a readonly projection of this property.
     */
    template<aui::invocable<const T&> Projection>
    [[nodiscard]]
    auto readonlyProjection(Projection&& projection) noexcept {
        return aui::detail::property::makeReadonlyProjection(*this, std::forward<Projection>(projection));
    }

    /**
     * @brief Makes a bidirectional projection of this property.
     */
    template<aui::invocable<const T&> ProjectionRead,
             aui::invocable<const std::invoke_result_t<ProjectionRead, T>&> ProjectionWrite>
    [[nodiscard]]
    auto bidirectionalProjection(ProjectionRead&& projectionRead, ProjectionWrite&& projectionWrite) noexcept {
        return aui::detail::property::makeBidirectionalProjection(*this,
                                                                  std::forward<ProjectionRead>(projectionRead),
                                                                  std::forward<ProjectionWrite>(projectionWrite));
    }

    /**
     * @brief Makes a bidirectional projection of this property (by a single aui::lambda_overloaded).
     */
    template<aui::detail::property::ProjectionBidirectional<T> Projection>
    [[nodiscard]]
    auto bidirectionalProjection(Projection&& projectionBidirectional) noexcept {
        return aui::detail::property::makeBidirectionalProjection(*this, projectionBidirectional);
    }
};
static_assert(AAnyProperty<AProperty<int>>, "AProperty does not conform AAnyProperty concept");

/**
 * @brief TODO
 * @ingroup property_system
 * @details
 * # Performance considerations
 * APropertyDef [does not involve](https://godbolt.org/z/cYTrc3PPf ) extra runtime overhead between assignment and
 * getter/setter.
 */
template <typename M, typename Getter, typename Setter, typename SignalArg>
struct APropertyDef {
    const M* base;
    using Model = M;
    Getter get;
    Setter set;
    using GetterReturnT = decltype(std::invoke(get, base));
    using Underlying = std::decay_t<GetterReturnT>;
    const emits<SignalArg>& changed;

    // this ctor effectively prohibits designated initialization, i.e., this one is not possible:
    //
    // auto size() const {
    //     return APropertyDef {
    //         .base = this,
    //         .get = &AView::mSize,
    //         .set = &AView::setSize,
    //         .changed = mSizeChanged,
    //     };
    // }
    //
    // deduction in designated initializers is relatively recent feature.
    APropertyDef(const M* base, Getter get, Setter set, const emits<SignalArg>& changed)
      : base(base), get(std::move(get)), set(std::move(set)), changed(changed) {}

    template <aui::convertible_to<Underlying> U>
    APropertyDef& operator=(U&& u) {
        std::invoke(set, const_cast<Model*>(base), std::forward<U>(u));
        return *this;
    }

    [[nodiscard]]
    GetterReturnT value() const noexcept {
        return std::invoke(get, base);
    }

    [[nodiscard]]
    GetterReturnT operator*() const noexcept {
        return std::invoke(get, base);
    }

    [[nodiscard]]
    const Underlying* operator->() const noexcept {
        return &std::invoke(get, base);
    }

    [[nodiscard]] operator GetterReturnT() const noexcept { return std::invoke(get, base); }

    [[nodiscard]]
    M* boundObject() const {
        return const_cast<M*>(base);
    }

    /**
     * @brief Makes a callable that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(std::move(*this));
    }

    /**
     * @brief Makes a readonly projection of this property.
     */
    template <aui::invocable<const Underlying&> Projection>
    [[nodiscard]]
    auto readonlyProjection(Projection&& projection) noexcept {
        return aui::detail::property::makeReadonlyProjection(std::move(*this), std::forward<Projection>(projection));
    }

    /**
     * @brief Makes a bidirectional projection of this property.
     */
    template <
        aui::invocable<const Underlying&> ProjectionRead,
        aui::invocable<const std::invoke_result_t<ProjectionRead, Underlying>&> ProjectionWrite>
    [[nodiscard]]
    auto bidirectionalProjection(ProjectionRead&& projectionRead, ProjectionWrite&& projectionWrite) noexcept {
        return aui::detail::property::makeBidirectionalProjection(
            *this, std::forward<ProjectionRead>(projectionRead), std::forward<ProjectionWrite>(projectionWrite));
    }

    /**
     * @brief Makes a bidirectional projection of this property (by a single aui::lambda_overloaded).
     */
    template <aui::detail::property::ProjectionBidirectional<Underlying> Projection>
    [[nodiscard]]
    auto bidirectionalProjection(Projection&& projectionBidirectional) noexcept {
        return aui::detail::property::makeBidirectionalProjection(*this, projectionBidirectional);
    };
};

// binary operations for properties.
template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator==(const Lhs& lhs, Rhs&& rhs) requires requires { *lhs == rhs; } {
    return *lhs == std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator!=(const Lhs& lhs, Rhs&& rhs) requires requires { *lhs != rhs; } {
    return *lhs != std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator+(const Lhs& lhs, Rhs&& rhs) requires requires { *lhs + rhs; } {
    return *lhs + std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
inline decltype(auto) operator+=(Lhs& lhs, Rhs&& rhs) requires requires { *lhs += rhs; } {
    return *lhs = *lhs + std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator-(const Lhs& lhs, Rhs&& rhs) requires requires { *lhs - rhs; } {
    return *lhs - std::forward<Rhs>(rhs);
}

// simple check above operators work.
static_assert(requires { AProperty<int>() + 1; });



// UNCOMMENT THIS to test bidirectionalProjection

static_assert(requires (AProperty<int>& intProperty) {
    { intProperty.bidirectionalProjection(aui::lambda_overloaded {
      [](int) -> AString { return ""; },
      [](const AString&) -> int { return 0; },
    }).value() } -> aui::convertible_to<AString>;

    { intProperty.bidirectionalProjection(aui::lambda_overloaded {
        [](int) -> AString { return ""; },
        [](const AString&) -> int { return 0; },
    }) = "AString" };


    { intProperty.bidirectionalProjection(aui::lambda_overloaded {
        [](int) -> AString { return ""; },
        [](const AString&) -> int { return 0; },
    }) };

    { intProperty.bidirectionalProjection(aui::lambda_overloaded {
        [](int) -> AString { return ""; },
        [](const AString&) -> int { return 0; },
    }).assignment() } -> aui::invocable<AString>;
});
