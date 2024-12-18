/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Traits/serializable.h>
#include "AJson.h"

template<>
struct ASerializable<AJson> {
    API_AUI_JSON static void write(IOutputStream& os, const AJson& value);
    API_AUI_JSON static void read(IInputStream& is, AJson& dst);
};