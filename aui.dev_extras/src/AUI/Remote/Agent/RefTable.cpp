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

#include "RefTable.h"

#include <charconv>

#include "JsonRpc.h"

namespace aui::remote {

namespace {
AString format(std::uint64_t generation, std::uint64_t id) {
    return "v" + AString::number(static_cast<std::int64_t>(generation)) + ":" +
           AString::number(static_cast<std::int64_t>(id));
}
}   // namespace

AString RefTable::add(const _<AView>& view) {
    if (auto it = mByView.find(view.get()); it != mByView.end()) {
        const auto [id, generation] = it->second;
        if (generation == mGeneration) {
            if (auto entry = mEntries.find(id); entry != mEntries.end() && entry->second.view.lock() == view) {
                return format(generation, id);
            }
        }
    }

    const auto id = mNext++;
    mEntries[id] = Entry { view, mGeneration };
    mOrder.push_back(id);
    mByView[view.get()] = { id, mGeneration };

    while (mEntries.size() > mCapacity && !mOrder.empty()) {
        const auto oldest = mOrder.front();
        mOrder.pop_front();
        if (auto it = mEntries.find(oldest); it != mEntries.end()) {
            if (auto v = it->second.view.lock()) {
                if (auto byView = mByView.find(v.get()); byView != mByView.end() && byView->second.first == oldest) {
                    mByView.erase(byView);
                }
            }
            mEntries.erase(it);
        }
    }
    return format(mGeneration, id);
}

_<AView> RefTable::resolve(const AString& ref) const {
    auto fail = [&](const char* why) -> RpcException {
        return RpcException(rpc_code::STALE_REF, "ref " + ref + " is " + why,
                            "take a new snapshot (i.e. view.interactive) and use fresh refs");
    };

    const auto s = ref.toStdString();
    std::uint64_t generation = 0, id = 0;
    if (s.size() < 4 || s[0] != 'v') {
        throw fail("malformed");
    }
    const auto colon = s.find(':');
    if (colon == std::string::npos) {
        throw fail("malformed");
    }
    auto [p1, e1] = std::from_chars(s.data() + 1, s.data() + colon, generation);
    auto [p2, e2] = std::from_chars(s.data() + colon + 1, s.data() + s.size(), id);
    if (e1 != std::errc {} || p1 != s.data() + colon || e2 != std::errc {} || p2 != s.data() + s.size()) {
        throw fail("malformed");
    }

    const auto it = mEntries.find(id);
    if (it == mEntries.end() || it->second.generation != generation) {
        throw fail("unknown or forgotten");
    }
    auto view = it->second.view.lock();
    if (!view) {
        throw fail("gone: the view doesn't exist anymore");
    }
    return view;
}

}   // namespace aui::remote
