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

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>

class API_AUI_REMOTE_TOOLS AHotCodeReload : public AObject {
public:
    static AHotCodeReload& instance() {
        static AHotCodeReload instance;
        return instance;
    }

    void reload();

    emits<> patchBegin;
    emits<> patchEnd;

private:
    std::unordered_map<AString, void*> mSymbols = extractSymbols();
    std::unordered_map<AString, void*> extractSymbols();
    AVector<_<void>> mAllocatedPages;

    AHotCodeReload() {}
};
