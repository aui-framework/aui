//
// Created by alex2 on 09.12.2020.
//

#pragma once


#include <cstdint>
#include <AUI/Common/AVector.h>
#include <AUI/Common/AColor.h>
#include "Texture.h"

namespace GL {
    class API_AUI_VIEWS Texture1D: public Texture<TEXTURE_1D> {
    public:
        void tex1D(const AVector<AColor>& image);
        virtual ~Texture1D() = default;
    };
}