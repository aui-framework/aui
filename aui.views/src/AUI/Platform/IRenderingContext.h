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

#pragma once


#include "AWindowNativePtr.h"
#include "IRenderingContext.h"
#include <AUI/Common/AString.h>
#include <AUI/Enum/WindowStyle.h>
#include <AUI/Image/AImage.h>

class ABaseWindow;
class AWindow;

class API_AUI_VIEWS IRenderingContext {
public:
    struct API_AUI_VIEWS Init {
        AWindow& window;
        AString name;
        int width;
        int height;
        WindowStyle ws;
        AWindow* parent;

        void setRenderingContext(_unique<IRenderingContext>&& context) const;
    };
    virtual void init(const Init& init);
    virtual ~IRenderingContext() = default;
    virtual void destroyNativeWindow(ABaseWindow& window) = 0;

    virtual AImage makeScreenshot() = 0;

    virtual void beginPaint(ABaseWindow& window) = 0;
    virtual void endPaint(ABaseWindow& window) = 0;
    virtual void beginResize(ABaseWindow& window) = 0;
    virtual void endResize(ABaseWindow& window) = 0;
};