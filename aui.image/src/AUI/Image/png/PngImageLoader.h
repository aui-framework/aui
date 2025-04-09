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
// Created by alex2 on 26.08.2020.
//

#pragma once


#include <AUI/api.h>
#include <AUI/Image/StbImageLoader.h>

class PngImageLoader: public StbImageLoader {
public:
    bool matches(AByteBufferView buffer) override;

    API_AUI_IMAGE static void save(aui::no_escape<IOutputStream> outputStream, AImageView image);
};


