// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 29.12.2020.
//

#include <AUI/Render/RenderHints.h>
#include "Backdrop.h"
#include "AUI/Common/AMap.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Render/ARender.h"
#include "AUI/Render/IRenderer.h"
#include "IProperty.h"

void ass::prop::Property<ass::Backdrop>::renderFor(AView* view) {
    auto renderer = ARender::getRenderer()
    auto offscreenRenderingOpt = renderer->getOffscreenRendering();
    if (!offscreenRenderingOpt) {
        return;
    }
    auto& offscreenRendering = offscreenRenderingOpt.value().get();
    for (const auto& effect: mInfo.effects) {
        std::visit(aui::lambda_overloaded {
            [&](const Backdrop::GaussianBlur& i) {

  

                offscreenRendering.performOffscreenRendering(IRenderer::IOffscreenRendering::RenderTarget::OFFSCREEN0, [&] {
                    auto defaultTarget = offscreenRendering.getRenderTargetTexture(IRenderer::IOffscreenRendering::RenderTarget::DEFAULT);
                    
                });

            }
        }, effect);
    }

    IPropertyBase::renderFor(view);
}

bool ass::prop::Property<ass::Backdrop>::isNone() {
    return mInfo.effects.empty();
}

ass::prop::PropertySlot ass::prop::Property<ass::Backdrop>::getPropertySlot() const {
    return ass::prop::PropertySlot::BACKDROP;
}


