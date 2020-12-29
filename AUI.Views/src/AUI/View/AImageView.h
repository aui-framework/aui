//
// Created by alex2 on 27.08.2020.
//

#pragma once

#include "AView.h"

class API_AUI_VIEWS AImageView: public AView {
private:
    _<GL::Texture2D> mTexture;
    glm::ivec2 mImageSize = {10, 10};

public:
    explicit AImageView(const _<GL::Texture2D>& texture);
    explicit AImageView(const _<AImage>& img);
    AImageView();

    void render() override;

    int getContentMinimumWidth() override;
    int getContentMinimumHeight() override;

    bool consumesClick(const glm::ivec2& pos) override;

    void setSize(int width, int height) override;
};


