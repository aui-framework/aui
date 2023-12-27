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

#pragma once

#include <AUI/Common/AVector.h>
#include <variant>
#include "AUI/Reflect/AEnumerate.h"



AUI_ENUM_FLAG(ARenderContextFlags) {
    DEFAULT = 0b0,

    /**
     * @brief Controls AUI's smooth window resize feature. If set to false, window's would resize dirty, however,
     * performance is increased by 10-50% especially in games.
     */
    NO_SMOOTH = 0b1,

    /**
     * @brief Disables vsync, causing the machine render as frequently as possible.
     */
    NO_VSYNC = 0b10,
};

struct API_AUI_VIEWS ARenderingContextOptions {
public:

    struct DirectX11 {
        int majorVersion;
    };

    struct Vulkan {
    };

    struct OpenGL {
        int majorVersion = 2, minorVersion = 0;

        enum class Profile {
            CORE,
            COMPAT
        } profile = Profile::CORE;
    };

    struct Software {};

    using InitializationVariant =  std::variant<DirectX11,
            Vulkan,
            OpenGL,
            Software>;

    AVector<InitializationVariant> initializationOrder;

    ARenderContextFlags flags = ARenderContextFlags::DEFAULT;

    static void set(ARenderingContextOptions options) {
        inst() = std::move(options);
    }
    static const ARenderingContextOptions& get() noexcept {
        return inst();
    }

private:
    static ARenderingContextOptions& inst();
};

AUI_ENUM_VALUES(ARenderingContextOptions::OpenGL::Profile,
                ARenderingContextOptions::OpenGL::Profile::CORE,
                ARenderingContextOptions::OpenGL::Profile::COMPAT)
