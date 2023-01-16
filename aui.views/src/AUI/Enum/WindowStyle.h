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

#include <AUI/Util/EnumUtil.h>

AUI_ENUM_FLAG(WindowStyle)
{
    DEFAULT = 0,
    /**
    * @brief Window without minimize and maximize buttons.
    */
    NO_MINIMIZE_MAXIMIZE = 0x1,

    /**
    * @brief Disable window resize.
    */
    NO_RESIZE = 0x2,

    /**
    * @brief Typical dialog window.
    */
    MODAL = WindowStyle::NO_MINIMIZE_MAXIMIZE | WindowStyle::NO_RESIZE,

    /**
    * @brief Remove standard window decorators.
    */
    NO_DECORATORS = 0x4,

    /**
    * @brief Window for displaying system menu (dropdown, context menu)
    */
    SYS = 0x8,

    /**
    * @brief Enables transparency for this window, so it can be displayed as custom rounded shadowed rectangle.
    * TODO implement WS_TRANSPARENT. WinAPI: http://rsdn.org/article/opengl/layeredopengl.xml, X11: https://github.com/datenwolf/codesamples/blob/master/samples/OpenGL/x11argb_opengl/x11argb_opengl.c
    */
// WS_TRANSPARENT
};
