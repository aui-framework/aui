// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>
#include <AUI/Common/AString.h>

namespace aui::impl::json {

/**
 * @brief Thread-local stack of JSON path segments accumulated during deserialization.
 * @details Each segment is either a string (object key) or a size_t (array index).
 * The stack is maintained automatically via PathSegmentGuard — no user code changes needed.
 */
inline thread_local std::vector<std::variant<AString, size_t>> gPathStack;

/**
 * @brief RAII guard that pushes a path segment on construction and pops it on destruction.
 * @details Used inside Field::operator(), AJsonConv for ranges/maps, etc.
 */
struct PathSegmentGuard {
    explicit PathSegmentGuard(AString key) { gPathStack.emplace_back(std::move(key)); }
    explicit PathSegmentGuard(size_t index)   { gPathStack.emplace_back(index); }
    ~PathSegmentGuard() noexcept           { gPathStack.pop_back(); }
    PathSegmentGuard(const PathSegmentGuard&) = delete;
    PathSegmentGuard& operator=(const PathSegmentGuard&) = delete;
};

/**
 * @brief Formats the current path stack as a human-readable string, e.g. `messages[351].content`.
 */
[[nodiscard]] inline AString currentPath() {
    AString result;
    for (const auto& seg : gPathStack) {
        std::visit([&](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, size_t>) {
                result += '[' + AString::number(v) + ']';
            } else {
                if (!result.empty()) result += '.';
                result += v;
            }
        }, seg);
    }
    return result;
}


} // namespace aui::impl::json
