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
#include <AUI/Common/detail/property.h>

/**
 * @brief Basic easy-to-use property implementation containing T.
 * @ingroup property_system
 * @details
 * @experimental
 * `AProperty<T>` is a container holding an instance of `T`. You can assign a value to it with `operator=` and read
 * value with `value()` method or implicit conversion `operator T()`.
 *
 * See @ref property_system "property system" for usage examples.
 */
template <typename T>
struct AProperty: AObjectBase {
    static_assert(!std::is_reference_v<T>, "====================> AProperty: attempt to wrap a reference.");

    using Underlying = T;

    /**
     * @brief Stored value.
     * @details
     * This field stores AProperty's wrapped value. It is not recommended to use this in casual use; although there are
     * might be an use case to modify the value without notifying. You can use `notify()` to send a change notification.
     * Use at your own risk.
     */
    T raw{};

    /**
     * @brief Signal that notifies data changes.
     */
    emits<T> changed;

    AProperty()
        requires aui::default_initializable<T>
    = default;

    template <aui::convertible_to<T> U>
    AProperty(U&& value) noexcept(noexcept(T(std::forward<U>(value)))): raw(std::forward<U>(value)) {}

    AObjectBase* boundObject() {
        return this;
    }

    const AObjectBase* boundObject() const {
        return this;
    }

    AProperty(const AProperty& value): raw(value.raw) {
    }

    AProperty(AProperty&& value) noexcept: raw(std::move(value.raw)) {
        value.notify();
    }

    AProperty& operator=(const AProperty& value) {
        if (this == &value) {
            return *this;
        }
        operator=(value.raw);
        return *this;
    }

    AProperty& operator=(AProperty&& value) noexcept {
        if (this == &value) {
            return *this;
        }
        operator=(std::move(value.raw));
        value.notify();
        return *this;
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
        notify();
        return *this;
    }

    template <ASignalInvokable SignalInvokable>
    void operator^(SignalInvokable&& t) {
        t.invokeSignal(nullptr);
    }

    /**
     * @brief Notify observers that a change was occurred (no preconditions).
     * @details
     * In common, you won't need to use this function. AProperty is reevaluated automatically as soon as one updates the
     * value within property.
     *
     * If your scenario goes beyond @ref writeScope that explicitly defines modification scope within RAII scope, you
     * can modify the underlying value by accessing `AProperty::raw` and then call @ref notify to notify the observers
     * that value is changed.
     */
    void notify() {
        emit changed(this->raw);
    }

    [[nodiscard]]
    const T& value() const noexcept {
        aui::react::DependencyObserverRegistrar::addDependency(changed);
        return raw;
    }

    [[nodiscard]] operator const T&() const noexcept { return value(); }

    [[nodiscard]]
    const T* operator->() const noexcept {
        return &value();
    }

    [[nodiscard]]
    const T& operator*() const noexcept {
        return value();
    }

    /**
     * @return @copybrief aui::PropertyModifier See aui::PropertyModifier.
     */
    aui::PropertyModifier<AProperty> writeScope() noexcept {
        return { *this };
    }

    /**
     * @brief Makes a readonly @ref UIDataBindingTest_Label_via_declarative_projection "projection" of this property.
     */
    template<aui::invocable<const T&> Projection>
    [[nodiscard]]
    auto readProjected(Projection&& projection) const noexcept {
        return aui::detail::property::makeReadonlyProjection(*this, std::forward<Projection>(projection));
    }

    /**
     * @brief Makes a bidirectional @ref UIDataBindingTest_Label_via_declarative_projection "projection" of this property.
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
static_assert(AAnyProperty<AProperty<int>&>, "AProperty does not conform AAnyProperty concept");
static_assert(APropertyReadable<const AProperty<int>>, "const AProperty does not conform AAnyProperty concept");

template<typename T>
class aui::PropertyModifier<AProperty<T>> {
public:
    using Underlying = T;
    PropertyModifier(AProperty<T>& owner): mOwner(&owner) {}
    ~PropertyModifier() {
        if (mOwner == nullptr) {
            return;
        }
        mOwner->notify();
    }

    [[nodiscard]]
    Underlying& value() const noexcept {
        return mOwner->raw;
    }

    [[nodiscard]]
    Underlying* operator->() const noexcept {
        return &value();
    }

private:
    AProperty<T>* mOwner;
};


/**
 * @brief Property implementation to use with custom getter/setter.
 * @ingroup property_system
 * @details
 * @experimental
 * You can use this way if you are required to define custom behaviour on getter/setter. As a downside, you have to
 * write extra boilerplate code: define property, data field, signal, getter and setter checking equality. Also,
 * APropertyDef requires the class to derive `AObject`. Most of AView's properties are defined this way.
 *
 * See @ref property_system "property system" for usage examples.
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
    // deduction in designated initializers is a relatively recent feature.
    APropertyDef(const M* base, Getter get, Setter set, const emits<SignalArg>& changed)
      : base(base), get(std::move(get)), set(std::move(set)), changed(changed) {}

    template <aui::convertible_to<Underlying> U>
    const APropertyDef& operator=(U&& u) const { // NOLINT(*-unconventional-assign-operator)
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
     * @brief Makes a readonly @ref UIDataBindingTest_Label_via_declarative_projection "projection" of this property.
     */
    template <aui::invocable<const Underlying&> Projection>
    [[nodiscard]]
    auto readProjected(Projection&& projection) noexcept {
        return aui::detail::property::makeReadonlyProjection(std::move(*this), std::forward<Projection>(projection));
    }

    /**
     * @brief Makes a bidirectional @ref UIDataBindingTest_Label_via_declarative_projection "projection" of this property.
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

    /**
     * @brief Notify observers that a change was occurred (no preconditions).
     */
    void notify() {
        if (changed.hasOutgoingConnections()) {
            emit changed(this->value());
        }
    }

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
// note: sync this PropertyModifier.h
template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator==(const Lhs& lhs, Rhs&& rhs) {
    return *lhs == std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator!=(const Lhs& lhs, Rhs&& rhs) {
    return *lhs != std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator<<(const Lhs& lhs, Rhs&& rhs) {
    return *lhs << std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator>>(const Lhs& lhs, Rhs&& rhs) {
    return *lhs >> std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator<(const Lhs& lhs, Rhs&& rhs) {
    return *lhs < std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator>(const Lhs& lhs, Rhs&& rhs) {
    return *lhs > std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator<=(const Lhs& lhs, Rhs&& rhs) {
    return *lhs <= std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator>=(const Lhs& lhs, Rhs&& rhs) {
    return *lhs >= std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator+(const Lhs& lhs, Rhs&& rhs) {
    return *lhs + std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator-(const Lhs& lhs, Rhs&& rhs) {
    return *lhs - std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
inline decltype(auto) operator+=(Lhs& lhs, Rhs&& rhs)  {
    if constexpr (requires { *lhs += std::forward<Rhs>(rhs); }) {
        // const operator?
        return *lhs += std::forward<Rhs>(rhs);
    } else {
        return *lhs.writeScope() += std::forward<Rhs>(rhs);
    }
}

template<AAnyProperty Lhs, typename Rhs>
inline decltype(auto) operator+=(Lhs&& lhs, Rhs&& rhs) {
    return lhs = *lhs + std::forward<Rhs>(rhs);
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
