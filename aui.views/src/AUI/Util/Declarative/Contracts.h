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

#include <AUI/Util/Declarative/Concepts.h>

namespace declarative::contract {

/**
 * @brief Denotes a potentially reactive value in declarative contracts.
 * @tparam T Type of the value.
 * @ingroup reactive
 * @details
 * A helper class that allows you to declare a value that can be either a constant or a reactive expression.
 */
template <typename T>
struct In {
private:
    /**
     * @brief Marks we were devastated.
     * @details
     * Since we use std::move/std::forward heavily (to avoid unnecessary copies), we need to make sure that the user
     * didn't call bindTo() twice.
     */
    struct Devastated {};

    /**
     * @brief Stores non-propertized (a regular) value.
     */
    struct Constant {
        T value;
    };

    /**
     * @brief Stores [reactive] expression.
     * @details
     * A [reactive] expression which are typically created by @ref AUI_REACT macro.
     */
    struct ReactiveExpression {
        _<APropertyPrecomputed<T>> value;
    };

public:
    template <aui::convertible_to<T> F>
    In(F&& t) : mImpl(Constant { std::forward<F>(t) }) {
        static_assert(
            !AAnyProperty<F>,
            "====================> contract::In: An attempt to use a property as a value in a declarative contract.\n"
            "1. Explicitly express an intention to use a reactive value by using AUI_REACT macro, or\n"
            "2. Dereference the property via asterisk * to obtain current property value.\n");
    }

    In(aui::factory<T> auto&& t) : mImpl(ReactiveExpression { _new<APropertyPrecomputed<T>>(std::move(t)) }) {}

    template <APropertyWritable DestinationProperty>
    void bindTo(DestinationProperty&& destinationProperty) {
        std::visit(
            aui::lambda_overloaded {
              [&](Devastated&) { throw AException("an attempt to bindTo a property twice"); },
              [&](Constant& c) { destinationProperty = std::move(c.value); },
              [&](ReactiveExpression& c) {
                  auto& sourceProperty = *c.value;
                  AObject::connect(
                      sourceProperty, destinationProperty.boundObject(),
                      [keepAlive = std::move(c.value),
                       // Using a tuple here to store args... and decide whether to store a value or a reference
                       // depending on whether the args... are lvalue or rvalue.
                       destinationProperty = std::tuple<DestinationProperty>(std::forward<DestinationProperty>(
                           destinationProperty))](const T& v) { std::get<0>(destinationProperty) = v; });
              },
            },
            mImpl);
        mImpl = Devastated {};
    }

private:
    std::variant<Devastated, Constant, ReactiveExpression> mImpl;
};

template <typename... Args>
struct Slot {
    static_assert(
        (std::is_object_v<Args> && ...),
        "// ====================> contract::Slot: there's no effect of specifying of non value arguments for the "
        "signal."
        "Consider removing const and reference modifiers.");

    Slot() = default;

    template <aui::convertible_to<AObjectBase*> ObjectPtr, typename Invocable>
    Slot(ObjectPtr receiverObject, Invocable&& receiverSlot)
      : mSetup([slotDef = ASlotDef { receiverObject, std::forward<Invocable>(receiverSlot) }](emits<Args...>& signal) {
          AObject::connect(signal, slotDef);
      }) {}

    template <typename Invocable>
    Slot(Invocable&& invocable)
      : mSetup([invocable = std::forward<Invocable>(invocable)](emits<Args...>& signal) mutable {
          AObject::connect(signal, AObject::GENERIC_OBSERVER, std::move(invocable));
      }) {}

    void bindTo(emits<Args...>& signal) {
        if (!mSetup) {
            return;
        }
        mSetup(signal);
    }

private:
    std::function<void(emits<Args...>& signal)> mSetup;
};

}   // namespace declarative::contract
