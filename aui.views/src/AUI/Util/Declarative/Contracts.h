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
#include <AUI/Common/React.h>

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

    template <typename Expr>
    In(aui::react::Expression<Expr>&& reactiveExpression)
      : mImpl(ReactiveExpression { _new<APropertyPrecomputed<T>>(std::move(reactiveExpression.expression)) }) {}

    /**
     * @brief Binds to the slot of the object.
     * @param receiver pointer to AObject-based object that willing to observe this contract. Either reference, or
     *        raw pointer, or smart pointer.
     * @param slot  Either pointer-to-member of receiver, or a lambda.
     * @details
     * If the contract holds a constant, the `invocable` will be called with stored value with no additional overhead.
     *
     * If the contract holds a result of `AUI_REACT`, `APropertyPrecomputed` will be bound to the receiver.
     *
     * This API was introduced to support old "retained-mode" UI:
     *
     * ```cpp
     * auto label = _new<ALabel>();
     * std::move(text).bindTo(AUI_SLOT(label)::setText);
     * ```
     */
    template <aui::detail::Receiver Object, typename Slot>
    void bindTo(const Object& receiver, Slot&& slot) && {
        std::visit(
            aui::lambda_overloaded {
              [&](Devastated&) { throw AException("an attempt to bindTo a property twice"); },
              [&](Constant& c) { std::invoke(aui::detail::dispatchSlot(receiver, std::forward<Slot>(slot)), std::move(c.value)); },
              [&](ReactiveExpression& c) {
                  auto& sourceProperty = *c.value;
                  AObject::connect(sourceProperty, receiver, [keepMeAlive = std::move(c.value), original = aui::detail::dispatchSlot(receiver, std::forward<Slot>(slot))](const T& value) {
                      std::invoke(original, value);
                  });
              },
            },
            mImpl);
        mImpl = Devastated {};
    }

    template <aui::detail::Receiver Object, typename Slot>
    void bindTo(const Object& receiver, Slot&& slot) const & {
        std::visit(
            aui::lambda_overloaded {
              [&](const Devastated&) { throw AException("an attempt to bindTo a property twice"); },
              [&](const Constant& c) { std::invoke(aui::detail::dispatchSlot(receiver, std::forward<Slot>(slot)), c.value); },
              [&](const ReactiveExpression& c) {
                  auto& sourceProperty = *c.value;
                  AObject::connect(sourceProperty, receiver, [keepMeAlive = c.value, original = aui::detail::dispatchSlot(receiver, std::forward<Slot>(slot))](const T& value) {
                      std::invoke(original, value);
                  });
              },
            },
            mImpl);
    }

    const T& value() const {
        return std::visit(
            aui::lambda_overloaded {
              [&](const Devastated&) -> const T& { throw AException("an attempt to dereference a property twice"); },
              [&](const Constant& c) -> const T& { return c.value; },
              [&](const ReactiveExpression& c) -> const T& { return **c.value; },
            }, mImpl
        );
    }

    const T& operator*() const { return value(); }

    const T* operator->() const { return &value(); }

private:
    std::variant<Devastated, Constant, ReactiveExpression> mImpl;
};

}   // namespace declarative::contract
