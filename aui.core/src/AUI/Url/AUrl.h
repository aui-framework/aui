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

#include "AUI/Common/AString.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include <AUI/Common/AMap.h>

#include <utility>

/**
 * @brief Uniform Resource Locator implementation.
 * @ingroup core
 * @details
 * Handles standard web url (schema://host:port/path).
 *
 * Empty schema is mapped to the assets file. In example, ":icon.svg" references to `icon.svg` file in your assets.
 * See aui_compile_assets() and [examples](https://github.com/aui-framework/aui/tree/master/examples/ui/views)
 * of asset usage.
 */
class API_AUI_CORE AUrl {
public:
    using Resolver = std::function<_unique<IInputStream>(const AUrl&)>;

    AUrl(AString full);

    inline AUrl(const char* full) : AUrl(AString(full)) {}

    AUrl(AString schema, AString path) : mSchema(std::move(schema)), mPath(std::move(path)) {}

    static AUrl file(const AString& file) {
        return {"file", file};
    }

    [[nodiscard]]
    _unique<IInputStream> open() const;

    [[nodiscard]]
    const AString& path() const noexcept {
        return mPath;
    }

    [[nodiscard]]
    const AString& schema() const noexcept {
        return mSchema;
    }

    [[nodiscard]]
    AString full() const {
        return mSchema + "://" + mPath;
    }

    bool operator<(const AUrl& u) const {
        return full() < u.full();
    }

    static void registerResolver(const AString& protocol, Resolver resolver);

private:
    AString mSchema;
    AString mPath;

    static AMap<AString, AVector<AUrl::Resolver>>& resolvers();
};


inline AUrl operator""_url(const char* input, size_t s) {
    return AUrl(AString(input, s));
}