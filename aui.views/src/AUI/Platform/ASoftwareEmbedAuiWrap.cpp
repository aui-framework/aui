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
// Created by Alex2772 on 1/24/2022.
//

#include "ASoftwareEmbedAuiWrap.h"
#include <AUI/Software/SoftwareRenderer.h>

ASoftwareEmbedAuiWrap::ASoftwareEmbedAuiWrap() {
    windowInit(aui::ptr::unique(mContext = new SoftwareRenderingContext));
//    ctx.render.setRenderer(std::make_unique<SoftwareRenderer>());
}

AImage ASoftwareEmbedAuiWrap::render(ARenderContext context) {
    windowMakeCurrent();
    AThread::processMessages();
    windowRender();
    return mContext->makeScreenshot();
}
