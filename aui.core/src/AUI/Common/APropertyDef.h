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

#include <AUI/Common/detail/property.h>

namespace aui::detail {
template<typename T>
struct OwningContainer {
    T object;
    T* operator->() {
        return &object;
    }
    T* operator->() const {
        return &object;
    }
    T& operator*() {
        return object;
    }
    T& operator*() const {}
};
}

/**
 * @brief Property implementation to use with custom getter/setter.
 * @ingroup property-system
 * @details
 * <!-- aui:experimental -->
 * You can use this way if you are required to define custom behaviour on getter/setter. As a downside, you have to
 * write extra boilerplate code: define property, data field, signal, getter and setter checking equality. Also,
 * APropertyDef requires the class to derive `AObject`. Most of AView's properties are defined this way.
 *
 * See [property system](property-system.md) for usage examples.
 *
 * ## Performance considerations
 *
 * APropertyDef [does not involve](https://godbolt.org/z/cYTrc3PPf ) extra runtime overhead between assignment and
 * getter/setter.
 *
 * <!-- aui:parse_tests aui.core/tests/PropertyDefTest.cpp -->
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
     * ```cpp
     * void setValue(int value) {
     *   if (mValue == value) {
     *     return;
     *   }
     *   mValue = value;
     *   emit mValueChanged(valueChanged);
     * }
     * ```
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
        aui::react::DependencyObserverScope::addDependency(changed);
        return std::invoke(get, base);
    }

    [[nodiscard]]
    GetterReturnT operator*() const noexcept {
        aui::react::DependencyObserverScope::addDependency(changed);
        return std::invoke(get, base);
    }

    [[nodiscard]]
    auto operator->() const noexcept {
        aui::react::DependencyObserverScope::addDependency(changed);
        if constexpr (std::is_reference_v<GetterReturnT>) {
            return &std::invoke(get, base);
        } else {
            // the only reason we do that is because compiler would say "bruh u can't take address of a temporary
            // object". so, we'll return a wrap that wraps -> instead, to extend lifetime of the object outside of scope
            // of this exact function.
            return aui::detail::OwningContainer<GetterReturnT>{ std::invoke(get, base) };
        }
    }

    [[nodiscard]] operator GetterReturnT() const noexcept {
        aui::react::DependencyObserverScope::addDependency(changed);
        return std::invoke(get, base);
    }

    [[nodiscard]]
    M* boundObject() const {
        return const_cast<M*>(base);
    }

    /**
     * @return @copybrief aui::PropertyModifier See aui::PropertyModifier.
     */
    aui::PropertyModifier<APropertyDef> writeScope() noexcept {
        return { std::move(*this) };
    }

    /**
     * @brief Notify observers that a change was occurred (no preconditions).
     */
    void notify() {
        if (changed.hasOutgoingConnections()) {
            emit changed(this->value());
        }
    }

    /**
     * @brief Makes ASlotDef that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(std::move(*this));
    }

private:
    friend class AObject;
};

// implementation of property modifier for APropertyDef (in comparison to AProperty) has to call getter, store a copy
// within PropertyModifier and call setter upon destruction.
// this behaviour described here - https://aui-framework.github.io/develop/structAPropertyDef.html#declaration
template <
    typename M, aui::invocable<M&> Getter, aui::invocable<M&, std::invoke_result_t<Getter, M&>> Setter,
    typename SignalArg>
class aui::PropertyModifier<APropertyDef<M, Getter, Setter, SignalArg>> {
public:
    using Property = APropertyDef<M, Getter, Setter, SignalArg>;
    using Underlying = std::decay_t<typename Property::Underlying>;
    PropertyModifier(Property owner): mOwner(std::move(owner)), mCopyOfValue(*mOwner) {}
    ~PropertyModifier() {
        mOwner = std::move(mCopyOfValue); // calls setter
    }

    [[nodiscard]]
    Underlying& value() const noexcept {
        return mCopyOfValue;
    }

    [[nodiscard]]
    Underlying* operator->() const noexcept {
        return &value();
    }

private:
    Property mOwner;
    mutable Underlying mCopyOfValue;
};
