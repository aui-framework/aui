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
 * @defgroup property_system Property System
 * @ingroup core
 * @brief Property System is a data binding mechanism based on [signal_slot] "signal-slot system".
 * @details
 * <!-- aui:experimental -->
 * AUI property system, a compiler-agnostic alternative to __property or [property]. Based on
 * [signal_slot] "signal-slot system" for platform-independent C++ development. Unlike Qt, AUI's properties don't
 * involve external tools (like `moc`). They are written in pure C++.
 *
 * !!! note
 *     This page is about presenting individual values. For lists, see [AForEachUI].
 *
 * AUI property system is relatively complex, as it involves a lot of features in a single place:
 *
 * 1. thread safe
 * 2. many-to-many relationships between objects
 * 3. optional data modification when passing values between objects (like STL projections)
 * 4. emitter can be either signal or property
 * 5. slot can be either lambda, method or property
 * 6. for the latter case, system must set up backward connection as well (including projection support)
 * 7. again, for the latter case, there's an option to make property-to-slot connection, where the "slot" is property's
 *    assignment operation
 * 8. 2 syntax variants: procedural (straightforward) and declarative
 * 9. three property variants: simple field (AProperty), custom getter/setter (APropertyDef) and custom evaluation
 *    (APropertyPrecomputed)
 * 10. some properties can be readonly
 * 11. propagating strong types' traits on views
 *
 * Learning curve is relatively flat, so be sure to
 * [ask questions and open issues](https://github.com/aui-framework/aui/issues) on our GitHub page.
 *
 * <!-- aui:parse_tests aui.uitests/tests/UIDataBindingTest.cpp -->
 */

/**
 * @brief Observable container of `T`.
 * @ingroup property_system
 * @details
 *
 * <!-- aui:experimental -->
 *
 * `AProperty<T>` is a container holding an instance of `T`. You can assign a value to it with `operator=` and read
 * value with `value()` method or implicit conversion `operator T()`.
 *
 * See [property system](property_system.md) for usage examples.
 *
 * <!-- aui:parse_tests aui.core/tests/PropertyTest.cpp -->
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
     * If your scenario goes beyond [AProperty::writeScope] that explicitly defines modification scope within RAII
     * scope, you can modify the underlying value by accessing `AProperty::raw` and then call [AProperty::notify] to
     * notify the observers value is changed.
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
     * @return aui::PropertyModifier of this property.
     */
    aui::PropertyModifier<AProperty> writeScope() noexcept {
        return { *this };
    }

    /**
     * @brief Makes a readonly [projection](property-system.md#UIDataBindingTest_Label_via_declarative_projection) of this property.
     */
    template<aui::invocable<const T&> Projection>
    [[nodiscard]]
    auto readProjected(Projection&& projection) const noexcept {
        return aui::detail::property::makeReadonlyProjection(*this, std::forward<Projection>(projection));
    }

    /**
     * @brief Makes a bidirectional [projection](property-system.md#UIDataBindingTest_Label_via_declarative_projection) of this property.
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

    template<typename Rhs>
    decltype(auto) operator[](Rhs&& rhs) const {
        return raw[std::forward<Rhs>(rhs)];
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

// implementation of property modifier for AProperty is relatively straightforward (in comparison to APropertyDef) since
// we have access to raw value. we just need to notify the property upon destruction
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

// simple check operators defined in detail/property.h work.
static_assert(requires { AProperty<int>() + 1; });

#include <AUI/Common/APropertyDef.h> // just for convenience
