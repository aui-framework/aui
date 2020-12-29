//
// Created by alex2 on 23.10.2020.
//

#pragma once


#include <AUI/Image/IDrawable.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Image/AImage.h>
#include <AUI/GL/Texture2D.h>

class ImageDrawable: public IDrawable {
private:
    GL::Texture2D mTexture;
    glm::ivec2 mSize;

public:
    explicit ImageDrawable(const _<AImage> image);
    virtual ~ImageDrawable();

    void draw(const glm::ivec2& size) override;

    glm::ivec2 getSizeHint() override;
};


