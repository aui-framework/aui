//
// Created by alex2 on 27.08.2020.
//

#pragma once

#include "AView.h"

class API_AUI_VIEWS AImageView: public AView {
private:
    _<GL::Texture> mTexture;
    glm::ivec2 mImageSize = {10, 10};

public:
    explicit AImageView(const _<GL::Texture>& texture);
    explicit AImageView(const _<AImage>& img);

    void render() override;

    int getContentMinimumWidth() override;
    int getContentMinimumHeight() override;

    void setSize(int width, int height) override;
};


