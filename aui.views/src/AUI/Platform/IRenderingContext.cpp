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
// Created by Alex2772 on 12/12/2021.
//

#include "IRenderingContext.h"

#include <AUI/Render/IRenderer.h>
#include <AUI/Platform/AWindow.h>

void IRenderingContext::Init::setRenderingContext(_unique<IRenderingContext>&& context) const {
    window.mRenderingContext = std::forward<_unique<IRenderingContext>>(context);
}
