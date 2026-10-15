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

#include <memory>

#include <AUI/Common/AColor.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Traits/strings.h>

class AView;
class ALabel;

/**
 * @brief Abstract base for a single inspectable property value.
 * @ingroup views
 * @details
 * Provides two rendering paths:
 *  - `renderAsString()` — plain text, suitable for CLI and agentic/AI profilers.
 *  - `renderAsView()` — rich view for the GUI devtools panel; default implementation wraps
 *    `renderAsString()` in an `ALabel`.
 *
 * The primary implementation is `AInspectableValue<T>`, which uses `fmt::formatter<T>` for
 * `renderAsString()`. Custom rendering can be added by specialising `AInspectableValue<T>`.
 */
struct API_AUI_VIEWS IInspectableValueBase {
    virtual ~IInspectableValueBase() = default;

    /**
     * @brief Renders the value as a plain string (CLI / agentic AI profiler path).
     */
    virtual AString renderAsString() const = 0;

    /**
     * @brief Renders the value as a view (GUI devtools path).
     * @details Default implementation returns `_new<ALabel>(renderAsString())`.
     */
    virtual AArc<AView> renderAsView() const;

};

/**
 * @brief Typed storage for an inspectable property value.
 * @tparam T stored value type. Must have a `fmt::formatter<T>` specialisation unless
 *           `AInspectableValue<T>` is explicitly specialised.
 * @ingroup views
 */
template <typename T>
struct AInspectableValue : IInspectableValueBase {
    T value;

    template <typename U>
    explicit AInspectableValue(U&& v) : value(std::forward<U>(v)) {}

    AString renderAsString() const override {
        return "{}"_format(value);
    }

};

/**
 * @brief Value-semantic wrapper for a single typed inspectable property.
 * @ingroup views
 * @details
 * Move-only wrapper around `std::unique_ptr<IInspectableValueBase>`. Constructed implicitly from
 * any value type `T` — this creates an `AInspectableValue<T>` internally.
 *
 * Example:
 * ```cpp
 * IInspectable::DebugInspectorInfo info;
 * info.push_back({ "Position", mPosition });   // glm::ivec2 — uses fmt::formatter
 * info.push_back({ "Color",    mColor });       // AColor   — uses specialised renderAsView()
 * ```
 */
class API_AUI_VIEWS AInspectable {
public:
    AInspectable() = delete;
    AInspectable(AInspectable&&) = default;
    AInspectable& operator=(AInspectable&&) = default;

    template <typename T>
    /*implicit*/ AInspectable(T&& v)  // NOLINT(google-explicit-constructor)
        : mValue(std::make_unique<AInspectableValue<std::decay_t<T>>>(std::forward<T>(v))) {}

    [[nodiscard]] IInspectableValueBase* get() noexcept { return mValue.get(); }
    [[nodiscard]] const IInspectableValueBase* get() const noexcept { return mValue.get(); }

    /**
     * @brief Plain-text rendering (CLI / agentic AI profiler path).
     */
    [[nodiscard]] AString renderAsString() const { return mValue->renderAsString(); }

    /**
     * @brief Rich view rendering (GUI devtools path).
     */
    [[nodiscard]] AArc<AView> renderAsView() const { return mValue->renderAsView(); }

private:
    std::unique_ptr<IInspectableValueBase> mValue;
};

// ─── Specialisations ─────────────────────────────────────────────────────────
// Types whose renderAsView() needs more than a plain label.
// The actual implementations live in InspectorRenderers.cpp (aui.views).

template <>
struct API_AUI_VIEWS AInspectableValue<AString> : IInspectableValueBase {
    AString value;
    explicit AInspectableValue(AString v) : value(std::move(v)) {}
    AString renderAsString() const override { return value; }
    AArc<AView> renderAsView() const override;
};

template <>
struct API_AUI_VIEWS AInspectableValue<AColor> : IInspectableValueBase {
    AColor value;
    explicit AInspectableValue(AColor v);
    AString renderAsString() const override;
    AArc<AView> renderAsView() const override;
};
