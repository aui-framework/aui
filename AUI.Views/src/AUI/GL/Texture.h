/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <cstdint>

namespace GL {
    constexpr uint32_t TEXTURE_1D = 0x0DE0;
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
        static void setupMirroredRepeat();
        Texture(const Texture&) = delete;

        void bind(uint8_t index = 0);

        operator bool() const {
            return mTexture;
        }
        uint32_t getHandle() const {
            return mTexture;
        }
    };
}