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
// Created by Nelonn on 4/12/2026.
//

#include "AUtf8.hpp"

#include <AUI/Common/AStaticVector.h>
#include <simdutf.h>

bool AUtf8View::isValidUtf8() const noexcept {
    return simdutf::validate_utf8(str);
}

AUtf8View::size_type AUtf8View::length() const noexcept {
    return simdutf::count_utf8(data(), sizeBytes());
}

bool AUtf8View::startsWith(AChar prefix) const noexcept {
    auto utf8p = prefix.toUtf8();
    return str.starts_with(std::string_view(utf8p.data(), utf8p.size()));
}

bool AUtf8View::endsWith(AChar prefix) const noexcept {

}
