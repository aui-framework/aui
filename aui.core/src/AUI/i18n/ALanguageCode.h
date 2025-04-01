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

//
// Created by alex2 on 07.11.2020.
//

#pragma once


#include <AUI/Common/AString.h>

/**
 * @brief Represents a language code in ISO 639-1, for example, en-US or ru-RU
 * @ingroup core
 */
class API_AUI_CORE ALanguageCode {
private:
    char mGroup[2];
    char mSubGroup[2];
public:
    ALanguageCode(const AString& str);
    ALanguageCode(const char* str): ALanguageCode(AString(str)) {}

    [[nodiscard]]
    AString toString() const {
        return AString(mGroup[0]) + mGroup[1] + "-" + mSubGroup[0] + mSubGroup[1];
    }

    [[nodiscard]]
    AString getGroup() const {
        return AString(mGroup, mGroup + 2);
    }

    [[nodiscard]]
    AString getSubGroup() const {
        return AString(mSubGroup, mSubGroup + 2);
    }
};


