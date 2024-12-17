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

#include <cstring>
#include "SvgImageLoader.h"

#include "SvgImageFactory.h"

SvgImageLoader::SvgImageLoader()
{
}

bool SvgImageLoader::matches(AByteBufferView buffer)
{
	return memcmp(buffer.data(), "<?xml", 5) == 0 ||
           memcmp(buffer.data(), "<svg", 4) == 0;
}


_<IImageFactory> SvgImageLoader::getImageFactory(AByteBufferView buffer) {
    return _new<SvgImageFactory>(buffer);
}

_<AImage> SvgImageLoader::getRasterImage(AByteBufferView buffer) {
    return nullptr;
}
