// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by Alex2772 on 12/9/2021.
//

#include <AUI/Platform/CommonRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Util/kAUI.h"


void CommonRenderingContext::init(const Init& init) {
    IRenderingContext::init(init);
}

void CommonRenderingContext::destroyNativeWindow(ABaseWindow& window) {

}

void CommonRenderingContext::beginPaint(ABaseWindow& window) {
}

void CommonRenderingContext::endPaint(ABaseWindow& window) {
}
