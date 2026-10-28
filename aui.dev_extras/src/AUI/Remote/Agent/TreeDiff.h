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

#include <array>
#include <cstdint>

#include "AUI/Common/AMap.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/AVector.h"
#include "AUI/Json/AJson.h"
#include "AUI/api.h"

namespace aui::remote::tree_diff {

/**
 * @brief A view as the client of `frames.subscribe` sees it. Geometry is relative to the parent, `absX` and `absY` to
 * the window.
 */
struct Node {
    AString type;
    AOptional<AString> text;
    AOptional<AString> name;
    /// type of the layout of a container, i.e. `AHorizontalLayout`
    AOptional<AString> layout;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int absX = 0;
    int absY = 0;
    bool visible = true;
    /// `{left, top, right, bottom}`; omitted in json when all zero
    std::array<int, 4> padding {};
    std::array<int, 4> margin {};
    AVector<AString> children;   // refs

    [[nodiscard]] bool operator==(const Node& rhs) const noexcept = default;
};

/**
 * @brief A flat map of ref to Node.
 */
struct Tree {
    AString root;
    AMap<AString, Node> nodes;

    [[nodiscard]] bool operator==(const Tree& rhs) const noexcept = default;
};

/**
 * @brief The difference between two trees; this is the `tree` field of `frames.frame`.
 */
struct Diff {
    /**
     * @brief seq of the frame the diff is computed against; null if the diff is against nothing, so `upsert` is the
     * whole tree.
     */
    AOptional<std::int64_t> base;
    AString root;
    AMap<AString, Node> upsert;
    AVector<AString> remove;
};

/**
 * @brief The full tree: `base` is null and `upsert` holds every node.
 */
API_AUI_DEV_EXTRAS Diff diff(const Tree& current);

/**
 * @brief Computes what changed from `base` to `current`.
 * @details
 * `upsert` holds every node that is new or differs (properties or the list of children) from `base`; the whole node is
 * replaced. `remove` holds refs of nodes that are in `base` but not in `current`.
 * @param baseSeq seq of the frame `base` came from; ends up in `Diff::base`
 */
API_AUI_DEV_EXTRAS Diff diff(const Tree& base, std::int64_t baseSeq, const Tree& current);

/**
 * @brief Applies the diff to `base`; `apply(diff(a, seq, b), a) == b`.
 * @param base ignored if `diff.base` is empty
 */
API_AUI_DEV_EXTRAS Tree apply(const Diff& diff, const Tree& base);

API_AUI_DEV_EXTRAS AJson toJson(const Node& node);
API_AUI_DEV_EXTRAS Node nodeFromJson(const AJson& json);

/**
 * @brief `{ base, root, upsert, remove }`.
 */
API_AUI_DEV_EXTRAS AJson toJson(const Diff& diff);
API_AUI_DEV_EXTRAS Diff diffFromJson(const AJson& json);

}   // namespace aui::remote::tree_diff
