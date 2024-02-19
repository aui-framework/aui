// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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
// Created by Alex2772 on 12/7/2021.
//

#include <AUI/GL/gl.h>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/GL/GLDebug.h>

#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/GL/State.h>


void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    ARender::setRenderer(mRenderer = ourRenderer());
}

void OpenGLRenderingContext::destroyNativeWindow(ABaseWindow& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void OpenGLRenderingContext::beginPaint(ABaseWindow& window) {
    beginFramebuffer(window.getSize());
    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::beginResize(ABaseWindow& window) {
}

void OpenGLRenderingContext::endResize(ABaseWindow& window) {

}

void OpenGLRenderingContext::endPaint(ABaseWindow& window) {
    endFramebuffer();
    mRenderer->endPaint();
    CommonRenderingContext::endPaint(window);
}

OpenGLRenderingContext::~OpenGLRenderingContext() {
}

AImage OpenGLRenderingContext::makeScreenshot() {
    // stub
    return AImage();
}
