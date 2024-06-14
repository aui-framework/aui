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

//
// Created by dervisdev on 1/11/2023.
//

#include "GifImageLoader.h"
#include "GifImageFactory.h"

bool GifImageLoader::matches(AByteBufferView buffer) {
    return memcmp(buffer.data(), "GIF87a", 6) == 0 ||
            memcmp(buffer.data(), "GIF89a", 6) == 0;
}

_<IImageFactory> GifImageLoader::getImageFactory(AByteBufferView buffer) {
    return _new<GifImageFactory>(buffer);
}

_<AImage> GifImageLoader::getRasterImage(AByteBufferView buffer) {
    return nullptr;
}