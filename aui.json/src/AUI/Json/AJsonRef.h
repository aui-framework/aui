// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <AUI/Common/AString.h>

class API_AUI_JSON AJson;


/**
 * @brief Path-tracking accessor returned by AJson::at(key) / AJson::at(index).
 * @details Accumulates path segments lazily; pushes them all onto gPathStack only
 * when a terminal method (asString, asInt, …) is called, so the path is visible
 * to as<T>() at throw time.
 */
class API_AUI_JSON AJsonRef {
public:
    using Segment = std::variant<AString, size_t>;

    AJsonRef(const AJson& node, std::vector<Segment> path)
        : mNode(&node), mPath(std::move(path)) {}

    // Extend the chain with a string key
    [[nodiscard]] AJsonRef at(const AString& key) const;
    // Extend the chain with an integer index
    [[nodiscard]] AJsonRef at(int index) const;

    // Terminal accessors — defined in AJson.cpp where AJson is fully known
    [[nodiscard]] AString asString() const;
    [[nodiscard]] int     asInt()    const;
    [[nodiscard]] double  asNumber() const;
    [[nodiscard]] bool    asBool()   const;

    // Implicit conversion to const AJson& for advanced use
    [[nodiscard]] operator const AJson&() const;

private:
    const AJson* mNode;
    std::vector<Segment> mPath;

    // Navigate to the node following mPath (without touching gPathStack)
    [[nodiscard]] const AJson& navigate() const;
};
