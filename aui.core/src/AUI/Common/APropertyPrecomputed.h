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

#include "AObjectBase.h"
#include "ASignal.h"
#include "AUI/Util/AEvaluationLoopException.h"
#include <AUI/Common/detail/property.h>
#include <AUI/Common/React.h>

/**
 * @brief Readonly property that holds a value computed by an expression.
 * @ingroup property-system
 * @ingroup reactive
 * @details
 * <!-- aui:experimental -->
 * `APropertyPrecomputed<T>` is a readonly property similar to `AProperty<T>`. It holds an instance of `T` as well.
 * Its value is determined by the C++ function specified in its constructor, typically a C++ lambda expression.
 *
 * See [property system](property-system.md) for usage info.
 *
 * Despite properties offer [projection methods](property-system.md#UIDataBindingTest_Label_via_declarative_projection), you might
 * want to track and process values of several properties.
 *
 * `APropertyPrecomputed<T>` is a readonly property similar to `AProperty<T>`. It holds an instance of `T` as well.
 * Its value is determined by a [reactive](aui::react) expression specified in `APropertyPrecomputed<T>`'s
 * constructor, typically a C++ lambda.
 *
 * It's convenient to access values from another properties inside the expression. The properties accessed during
 * invocation of the expression are tracked behind the scenes so they become dependencies of `APropertyPrecomputed`
 * automatically. If one of the tracked properties fires `changed` signal, `APropertyPrecomputed` invalidates its
 * `T`. `APropertyPrecomputed` follows [lazy semantics](aui::lazy) so the expression is re-evaluated and the new
 * result is applied to `APropertyPrecomputed` as soon as the latter is accessed for the next time.
 *
 * In other words, it allows to specify relationships between different object properties and reactively update
 * `APropertyPrecomputed` value whenever its dependencies change. `APropertyPrecomputed<T>` is somewhat similar to
 * [Qt Bindable Properties](https://doc.qt.io/qt-6/bindableproperties.html).
 *
 * `APropertyPrecomputed` is a readonly property, hence you can't update its value with assignment. You can get its
 * value with `value()` method or implicit conversion `operator T()` as with other properties.
 *
 * <!-- aui:parse_tests aui.core/tests/PropertyPrecomputed.cpp -->
 */
template<typename T>
struct APropertyPrecomputed final : aui::react::DependencyObserver {
    using Underlying = T;
    using Factory = std::function<T()>;

    template<aui::factory<T> Factory>
    APropertyPrecomputed(Factory&& expression): mCurrentValue([this, expression = std::forward<Factory>(expression)] { // NOLINT(*-explicit-constructor)
      aui::react::DependencyObserverRegistrar r(*this);
      return expression();
    }) {

    }

    APropertyPrecomputed(const APropertyPrecomputed&) = delete;
    APropertyPrecomputed(APropertyPrecomputed&&) noexcept = delete;

    template <ASignalInvokable SignalInvokable>
    void operator^(SignalInvokable&& t) {
        t.invokeSignal(nullptr);
    }

    /**
     * @brief Marks this precomputed property to be reevaluated.
     * @details
     * In common, you won't need to use this function. APropertyPrecomputed is reevaluated automatically as soon as one
     * updates a property expression depends on.
     */
    void invalidate() override {
        mCurrentValue.reset();
        if (this->changed) {
            if (this->changed.isAtSignalEmissionState()) {
                mCurrentValue.setEvaluationLoopTrap();
                return;
            }
            emit this->changed(value());
        }
    }

    const AObjectBase* boundObject() const {
        return this;
    }

    [[nodiscard]]
    const T& value() const {
        aui::react::DependencyObserverRegistrar::addDependency(changed);
        return mCurrentValue;
    }

    [[nodiscard]]
    operator const T&() const {
        return value();
    }

    [[nodiscard]]
    const T& operator*() const {
        return value();
    }

    [[nodiscard]]
    const T* operator->() const noexcept {
        return &value();
    }

signals:
    emits<T> changed;

private:
    aui::lazy<T> mCurrentValue;
};

template<aui::invocable<> Factory>
APropertyPrecomputed(Factory&& f) -> APropertyPrecomputed<std::decay_t<std::invoke_result_t<Factory>>>;

static_assert(APropertyReadable<APropertyPrecomputed<int>>, "APropertyPrecomputed must be a APropertyReadable");

namespace aui::react {

/**
 * @brief Reactive expression produced by AUI_REACT macro.
 */
template <aui::invocable F>
struct Expression {
    F expression;


    template <typename Object, typename Invocable>
    void bindTo(ASlotDef<Object, Invocable> destination) {
        aui::tuple_visitor<typename aui::lambda_info<Invocable>::args>::for_each_all([&]<typename ExtractedType>() {
            using T = std::decay_t<ExtractedType>;
            auto precomputed = _new<APropertyPrecomputed<T>>(std::move(expression));
            auto& precomputedValue = *precomputed; // extract reference to precomputed, because we'll std::move the
                                                   // shared_ptr
            AObject::connect(
                precomputedValue, destination.boundObject,
                [keepMeAlive = std::move(precomputed), invocable = std::move(destination.invocable)](const T& value) {
                    std::invoke(invocable, value);
                });
            });
    }
};
}


/**
 * @brief Explicitly denotes a [reactive](reactive.md) expression.
 * @ingroup useful_macros
 * @ingroup reactive
 * @details
 * AUI_REACT is a core component of AUI Framework's [reactive](reactive.md) reactive programming model. It's used to
 * create [reactive](reactive.md) expressions that automatically update UI elements when their dependent values
 * change.
 *
 * The expression is a C++ expression that depends on AProperty values:
 *
 * ```cpp
 * AUI_REACT(expression)
 * ```
 *
 * ## Basic example
 *
 * This creates a label that automatically updates when property `mCounter` changes:
 *
 * <!-- aui:snippet examples/7guis/counter/src/main.cpp counter -->
 *
 * ## Formatted label example
 *
 * <!-- aui:snippet examples/7guis/timer/src/main.cpp example -->
 *
 * ### Implementation details
 *
 * When used in declarative UI building, `AUI_REACT` creates an instance of `APropertyPrecomputed<T>` behind the scenes,
 * which:
 *
 * 1. Evaluates the expression initially.
 * 2. Sets up observers for all dependent properties.
 * 3. Re-evaluates when dependencies change.
 *
 * The macros itself consists of a lambda syntax with forced `[=]` capture and explicit `decltype(auto)` return type.
 * The lambda is wrapped with aui::react::Expression to be strongly typed.
 */
#define AUI_REACT(expression) ::aui::react::Expression { [=] { return (expression); } }
