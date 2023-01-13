//
// Created by dervisdev on 1/12/2023.
//
#pragma once

#include "IDrawable.h"
#include "AUI/Image/IImageFactory.h"
#include "AUI/Render/Render.h"

class AAnimatedDrawable : public IDrawable {
private:
    _<IImageFactory> mFactory;
public:
    explicit AAnimatedDrawable(_<IImageFactory> factory): mFactory (std::move(factory)) {}
    ~AAnimatedDrawable() = default;

    void draw(const Params &params) override;
    glm::ivec2 getSizeHint() override;
};

