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

namespace AStreams {
    inline void copyAll(aui::no_escape<IInputStream> is, aui::no_escape<IOutputStream> os, size_t bytes) {
        char buf[0x1000];
        while (bytes > 0) {
            size_t r = is->read(buf, (glm::min)(bytes, sizeof(buf)));
            if (r == 0) {
                throw AEOFException();
            }
            os->write(buf, r);
            bytes -= r;
        }
    }
}