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
private:
    friend class API_AUI_CORE ::AObject;

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
    auto readProjected =
        makeReadonlyProjection(std::forward<Property>(property), std::forward<ProjectionRead>(projectionRead));
    using PropertyReadProjection = decltype(readProjected);
    PropertyReadWriteProjection result(std::move(readProjected), std::forward<ProjectionWrite>(projectionWrite));
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
 * @brief Basic easy-to-use property implementation containing T.
 * @ingroup property_system
 * @details
 * `AProperty<T>` is a container holding an instance of `T`. You can assign a value to it with `operator=` and read
 * value with `value()` method or implicit conversion `operator T()`.
 *
 * See @ref property_system "property system" for more info.
 * @example
 * @code{cpp}
 * struct User {
 *   AProperty<AString> name;
 *   AProperty<AString> surname;
 * };
 *
 * // AProperty behaves like a class/struct data member:
 * User u;
 * u.name = "Hello";
 * EXPECT_EQ(u.name, "Hello");
 * @endcode
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
     * @brief Makes a readonly projection of this property.
     */
    template<aui::invocable<const T&> Projection>
    [[nodiscard]]
    auto readProjected(Projection&& projection) noexcept {
        return aui::detail::property::makeReadonlyProjection(*this, std::forward<Projection>(projection));
    }

    /**
     * @brief Makes a bidirectional projection of this property.
     */
    template<aui::invocable<const T&> ProjectionRead,
             aui::invocable<const std::invoke_result_t<ProjectionRead, T>&> ProjectionWrite>
    [[nodiscard]]
    auto biProjected(ProjectionRead&& projectionRead, ProjectionWrite&& projectionWrite) noexcept {
        return aui::detail::property::makeBidirectionalProjection(*this,
                                                                  std::forward<ProjectionRead>(projectionRead),
                                                                  std::forward<ProjectionWrite>(projectionWrite));
    }

    /**
     * @brief Makes a bidirectional projection of this property (by a single aui::lambda_overloaded).
     */
    template<aui::detail::property::ProjectionBidirectional<T> Projection>
    [[nodiscard]]
    auto biProjected(Projection&& projectionBidirectional) noexcept {
        return aui::detail::property::makeBidirectionalProjection(*this, projectionBidirectional);
    }

private:
    friend class AObject;
    /**
     * @brief Makes a callable that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(*this);
    }
};
static_assert(AAnyProperty<AProperty<int>>, "AProperty does not conform AAnyProperty concept");

/**
 * @brief Property implementation to use with custom getter/setter.
 * @ingroup property_system
 * @details
 * See @ref property_system "property system" for more info.
 *
 * @example
 * @code{cpp}
 * class User: public AObject {
 * public:
 *     auto name() const {
 *         return APropertyDef {
 *             this,
 *             &User::getName, // this works too: &User::mName
 *             &User::setName,
 *             mNameChanged,
 *         };
 *     }
 *
 * private:
 *     AString mName;
 *     emits<AString> mNameChanged;
 *
 *     void setName(AString name) {
 *         // APropertyDef requires us to emit
 *         // changed signal if value is actually
 *         // changed
 *         if (mName == name) {
 *             return;
 *         }
 *         mName = std::move(name);
 *         emit mNameChanged(mName);
 *     }
 *
 *     const AString& getName() const {
 *         return mName;
 *     }
 * };
 * @endcode
 *
 * # Performance considerations
 * APropertyDef [does not involve](https://godbolt.org/z/cYTrc3PPf ) extra runtime overhead between assignment and
 * getter/setter.
 */
template <
    typename M, aui::invocable<M&> Getter, aui::invocable<M&, std::invoke_result_t<Getter, M&>> Setter,
    typename SignalArg>
struct APropertyDef {
    /**
     * @brief AObject which this property belongs to.
     */
    const M* base;
    using Model = M;

    /**
     * @brief Getter. Can be pointer-to-member(function or field) or lambda.
     */
    Getter get;

    /**
     * @brief Setter. Can be pointer-to-member(function or field) or lambda.
     * @details
     * The setter implementation typically emits `changed` signal. If it is, it must emit changes only if value is
     * actually changed.
     * @code{cpp}
     * void setValue(int value) {
     *   if (mValue == value) {
     *     return;
     *   }
     *   mValue = value;
     *   emit mValueChanged(valueChanged);
     * }
     * @endcode
     */
    Setter set;
    using GetterReturnT = decltype(std::invoke(get, base));
    using Underlying = std::decay_t<GetterReturnT>;

    /**
     * @brief Reference to underlying signal emitting on value changes.
     */
    const emits<SignalArg>& changed;
    static_assert(aui::same_as<Underlying , std::decay_t<SignalArg>>, "different getter result and signal arg?");

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
        std::invoke(set, *const_cast<Model*>(base), std::forward<U>(u));
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
     * @brief Makes a readonly projection of this property.
     */
    template <aui::invocable<const Underlying&> Projection>
    [[nodiscard]]
    auto readProjected(Projection&& projection) noexcept {
        return aui::detail::property::makeReadonlyProjection(std::move(*this), std::forward<Projection>(projection));
    }

    /**
     * @brief Makes a bidirectional projection of this property.
     */
    template <
        aui::invocable<const Underlying&> ProjectionRead,
        aui::invocable<const std::invoke_result_t<ProjectionRead, Underlying>&> ProjectionWrite>
    [[nodiscard]]
    auto biProjected(ProjectionRead&& projectionRead, ProjectionWrite&& projectionWrite) noexcept {
        return aui::detail::property::makeBidirectionalProjection(
            std::move(*this), std::forward<ProjectionRead>(projectionRead),
            std::forward<ProjectionWrite>(projectionWrite));
    }

    /**
     * @brief Makes a bidirectional projection of this property (by a single aui::lambda_overloaded).
     */
    template <aui::detail::property::ProjectionBidirectional<Underlying> Projection>
    [[nodiscard]]
    auto biProjected(Projection&& projectionBidirectional) noexcept {
        return aui::detail::property::makeBidirectionalProjection(std::move(*this), projectionBidirectional);
    };

private:
    friend class AObject;
    /**
     * @brief Makes a callable that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(std::move(*this));
    }
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
    *lhs += std::forward<Rhs>(rhs);
    return lhs;
}

template<AAnyProperty Lhs, typename Rhs>
inline decltype(auto) operator+=(Lhs&& lhs, Rhs&& rhs) requires requires { lhs = *lhs + std::forward<Rhs>(rhs); } {
    return lhs = *lhs + std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator-(const Lhs& lhs, Rhs&& rhs) requires requires { *lhs - rhs; } {
    return *lhs - std::forward<Rhs>(rhs);
}

// simple check above operators work.
static_assert(requires { AProperty<int>() + 1; });


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

template <APropertyReadable T> struct fmt::formatter<T> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    auto format(T& c, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "{}", *c);
    }
};
