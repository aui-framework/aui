// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

//
// Created by alex2772 on 5/27/26.
//

#include "AJson.h"
#include "AJsonRef.h"
#include "Path.h"

const AJson& AJsonRef::navigate() const {
    const AJson* cur = mNode;
    for (const auto& seg : mPath) {
        std::visit([&](const auto& s) {
            using T = std::decay_t<decltype(s)>;
            if constexpr (std::is_same_v<T, AString>) {
                cur = &(*cur)[s];
            } else {
                cur = &(*cur)[static_cast<int>(s)];
            }
        }, seg);
    }
    return *cur;
}

AJsonRef AJsonRef::at(const AString& key) const {
    auto newPath = mPath;
    newPath.emplace_back(key);
    return AJsonRef{*mNode, std::move(newPath)};
}

AJsonRef AJsonRef::at(int index) const {
    auto newPath = mPath;
    newPath.emplace_back(static_cast<size_t>(index));
    return AJsonRef{*mNode, std::move(newPath)};
}

AJsonRef::operator const AJson&() const {
    return navigate();
}

namespace {
// RAII: push all path segments onto gPathStack, pop them on destruction
struct PathPusher {
    std::size_t n;
    explicit PathPusher(const std::vector<AJsonRef::Segment>& path) : n(path.size()) {
        for (const auto& seg : path) {
            std::visit([](const auto& s) { aui::impl::json::gPathStack.emplace_back(s); }, seg);
        }
    }
    ~PathPusher() noexcept {
        for (std::size_t i = 0; i < n; ++i) aui::impl::json::gPathStack.pop_back();
    }
};
} // namespace

AString AJsonRef::asString() const {
    PathPusher p{mPath};
    return navigate().asString();
}

int AJsonRef::asInt() const {
    PathPusher p{mPath};
    return navigate().asInt();
}

double AJsonRef::asNumber() const {
    PathPusher p{mPath};
    return navigate().asNumber();
}

bool AJsonRef::asBool() const {
    PathPusher p{mPath};
    return navigate().asBool();
}

