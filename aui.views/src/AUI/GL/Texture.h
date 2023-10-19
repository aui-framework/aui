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

#include <cstdint>

namespace gl {
    constexpr uint32_t TEXTURE_1D = 0x0DE1;
    constexpr uint32_t TEXTURE_2D = 0x0DE1;
    constexpr uint32_t TEXTURE_3D = 0x806F;


    template<unsigned TEXTURE_TARGET>
    class API_AUI_VIEWS Texture {
    private:
        uint32_t mTexture = 0;
    public:
        Texture();
        virtual ~Texture();
        static void setupNearest();
        static void setupLinear();
        static void setupClampToEdge();
        static void setupMirroredRepeat();
        Texture(const Texture&) = delete;

        void bind(uint8_t index = 0);
        static void unbind(uint8_t index = 0);

        operator bool() const {
            return mTexture;
        }
        uint32_t getHandle() const {
            return mTexture;
        }
    };
}