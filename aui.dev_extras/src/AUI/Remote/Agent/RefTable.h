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

#include <cstdint>
#include <deque>
#include <map>
#include <unordered_map>

#include "AUI/Common/AString.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/View/AView.h"
#include "AUI/api.h"

namespace aui::remote {

/**
 * @brief Gives views short names (refs) a remote client can pass back.
 * @details
 * A ref looks like `v3:17`: the generation and the number of the entry. Entries hold views weakly, so a ref never
 * keeps a view alive; resolving a ref of a view that is gone throws RpcException with STALE_REF. The same view gets
 * the same ref within a generation. Start a new generation (newGeneration) whenever the client takes a new snapshot of
 * the tree, so the refs of an older snapshot are recognizable as such.
 *
 * The table forgets the oldest entries when it grows beyond the capacity.
 *
 * Use from the UI thread only.
 */
class API_AUI_DEV_EXTRAS RefTable {
public:
    explicit RefTable(std::size_t capacity = 10'000) : mCapacity(capacity) {}

    /**
     * @brief Starts a new generation: views added afterwards get refs with the new generation number.
     */
    void newGeneration() noexcept { ++mGeneration; }

    [[nodiscard]] std::uint64_t generation() const noexcept { return mGeneration; }

    /**
     * @return the ref of the view
     */
    [[nodiscard]] AString add(const _<AView>& view);

    /**
     * @return the view by the ref
     * @throws RpcException with STALE_REF if the ref is malformed, unknown, forgotten, or its view is gone.
     */
    [[nodiscard]] _<AView> resolve(const AString& ref) const;

    [[nodiscard]] std::size_t size() const noexcept { return mEntries.size(); }

private:
    struct Entry {
        _weak<AView> view;
        std::uint64_t generation;
    };

    std::size_t mCapacity;
    std::uint64_t mGeneration = 1;
    std::uint64_t mNext = 1;
    std::map<std::uint64_t, Entry> mEntries;
    std::deque<std::uint64_t> mOrder;   // oldest first
    std::unordered_map<const AView*, std::pair<std::uint64_t /* id */, std::uint64_t /* generation */>> mByView;
};

}   // namespace aui::remote
