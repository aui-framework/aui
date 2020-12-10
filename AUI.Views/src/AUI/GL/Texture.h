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