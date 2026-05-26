/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Traits/values.h>
#include <AUI/Util/APool.h>
#include <AUI/Image/AImage.h>
#include <AUI/Render/IRendererBackend.h>
#include <AUI/Platform/SoftwareRenderingContext.h>
#include <AUI/ASS/Property/Backdrop.h>
#include <AUI/Render/SimpleTexturePacker.h>
#include <AUI/Common/ADeque.h>
#include <variant>
#include <span>
#include "SoftwareTexture.h"

class SoftwareRenderer : public IRendererBackend {
    friend class SoftwarePrerenderedString;
    friend class SoftwareMultiStringCanvas;
public:
    struct FontEntryData: aui::noncopyable {
        SoftwareRenderer* renderer;
        _<SoftwareTexture> texture;
        bool isTextureInvalid = true;
        Util::SimpleTexturePacker texturePacker;

        explicit FontEntryData(SoftwareRenderer* renderer): renderer(renderer), texture(_new<SoftwareTexture>()) {}
    };

    struct CharacterData {
        glm::vec4 uv;
    };

    SoftwareRenderer();

    // IRendererBackend implementation
    void solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, Blending blending) override;
    void gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, Blending blending) override;
    void texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, Blending blending) override;
    void solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, Blending blending) override;
    void gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, Blending blending) override;
    void texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, Blending blending) override;
    void rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, Blending blending) override;
    void roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, Blending blending) override;
    void boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, Blending blending) override;
    void boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, Blending blending) override;
    void glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, Blending blending) override;
    _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;
    void lines(const ADisplayList::Lines& v, const glm::mat4& transform, Blending blending) override;
    void points(const ADisplayList::Points& v, const glm::mat4& transform, Blending blending) override;
    void lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, Blending blending) override;
    void squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, Blending blending) override;

    void backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) override;
    void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override;

    _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override;
    void setWindow(ASurface* window) override;
    ASurface* getWindow() const noexcept override { return mWindow; }

    float getRenderScale() const noexcept override { return mRenderScale; }
    void setRenderScale(float renderScale) override { mRenderScale = renderScale; }

    void setAllowRenderToTexture(bool allow) override { mAllowRenderToTexture = allow; }
    bool allowRenderToTexture() const noexcept override { return mAllowRenderToTexture; }

    _<ITexture> createTexture(glm::u32vec2 size) override;
    _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;
    glm::mat4 getProjectionMatrix() const override;

protected:
    void putPixel(glm::ivec2 pos, AColor color, Blending blending = Blending::NORMAL);
    FontEntryData* getFontEntryData(const AFontStyle& fontStyle);

    SoftwareRenderingContext* mContext = nullptr;
    ASurface* mWindow = nullptr;
    float mRenderScale = 1.0f;
    bool mAllowRenderToTexture = true;
    ADeque<FontEntryData> mFontEntryData;
    ADeque<CharacterData> mCharData;
};
