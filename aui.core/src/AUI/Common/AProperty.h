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
 * See [property system](property_system) for usage examples.
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
     * If your scenario goes beyond [writeScope] that explicitly defines modification scope within RAII scope, you
     * can modify the underlying value by accessing `AProperty::raw` and then call [notify] to notify the observers
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
     * @brief Makes a readonly [projection](UIDataBindingTest_Label_via_declarative_projection) of this property.
     */
    template<aui::invocable<const T&> Projection>
    [[nodiscard]]
    auto readProjected(Projection&& projection) const noexcept {
        return aui::detail::property::makeReadonlyProjection(*this, std::forward<Projection>(projection));
    }

    /**
     * @brief Makes a bidirectional [projection](UIDataBindingTest_Label_via_declarative_projection) of this property.
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
