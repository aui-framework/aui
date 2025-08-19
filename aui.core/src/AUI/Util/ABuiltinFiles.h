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

#include "AUI/Core.h"
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include <optional>

class AString;

/**
 * @brief Filesystem for [aui-assets.md].
 */
class API_AUI_CORE ABuiltinFiles {
private:
    AMap<std::string_view, AByteBufferView> mBuffers;

    static ABuiltinFiles& inst();

    ABuiltinFiles() = default;

public:
    static void registerAsset(std::string_view path, const unsigned char* data, size_t size,
                              std::string_view programModule = AUI_PP_STRINGIZE(AUI_MODULE_NAME));

    static _unique<IInputStream> open(const AString& file);

    static bool contains(const AString& file);
};
