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


#include "AEmbedAuiWrap.h"

/**
 * @brief Embed AUI into OpenGL rendering context
 */
class API_AUI_VIEWS AGLEmbedAuiWrap: public AEmbedAuiWrap {
public:
    AGLEmbedAuiWrap();
    void render(ARenderContext context);
    void resetGLState();

};


