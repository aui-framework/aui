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

#include <AUI/Platform/SoftwareRenderingContext.h>
#include "RenderingContextX11.h"

class SoftwareRenderingContextX11: public SoftwareRenderingContext, public RenderingContextX11 {
public:
    ~SoftwareRenderingContextX11() override;
    void init(const Init& init) override;
    void destroyNativeWindow(ASurface& window) override;
    void endPaint(ASurface& window) override;

protected:
    void reallocate() override;

private:
    _<XImage> mXImage;
    std::unique_ptr<_XGC, void(*)(GC)> mGC = {nullptr, nullptr};

};