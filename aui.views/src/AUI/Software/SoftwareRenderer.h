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
#include <AUI/Common/ADeque.h>
#include <AUI/Render/FontAtlas.hpp>
#include <variant>
#include <span>
#include "SoftwareTexture.h"
#include <AUI/Render/IRenderViewToTexture.h>
#include <AUI/Render/ACanvas.hpp>

class SoftwareRenderViewToTexture;

class SoftwareRenderer : public IRendererBackend {
    friend class SoftwareRenderViewToTexture;
public:
    SoftwareRenderer();
    ~SoftwareRenderer() override = default;

    void solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, const APaint& paint) override;
    void roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, const APaint& paint) override;
    void boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, const APaint& paint) override;
    void boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, const APaint& paint) override;
    void glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, const APaint& paint) override;
    _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;
    _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;
    void lines(const ADisplayList::Lines& v, const glm::mat4& transform, const APaint& paint) override;
    void points(const ADisplayList::Points& v, const glm::mat4& transform, const APaint& paint) override;
    void lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, const APaint& paint) override;
    void squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, const APaint& paint) override;

    void backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) override;
    void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override;
void setWindow(ASurface* window) override;
ASurface* getWindow() const noexcept override { return mWindow; }

void setAllowRenderToTexture(bool allow) override { mAllowRenderToTexture = allow; }
bool allowRenderToTexture() const noexcept override { return mAllowRenderToTexture; }

_<ITexture> createTexture(glm::u32vec2 size, APixelFormat format = APixelFormat::RGBA_BYTE, TextureFilter filter = TextureFilter::LINEAR) override;
glm::mat4 getProjectionMatrix() const override;

const _<aui::AFontCache>& getFontCache() override { return mFontCache; }

void setMask(const _<ITexture>& mask, const glm::vec4& maskRect = glm::vec4(0.f)) override;

protected:
void putPixel(glm::ivec2 pos, AColor color, const APaint& paint);
void drawLine(glm::ivec2 p0, glm::ivec2 p1, float width, AColor color, const APaint& paint);

SoftwareRenderingContext* mContext = nullptr;
AImage* mRenderTarget = nullptr;
ASurface* mWindow = nullptr;
bool mAllowRenderToTexture = true;
ADeque<aui::FontAtlas> mFontEntryData;
ADeque<aui::CharacterData> mCharData;
_<aui::AFontCache> mFontCache;
};
class SoftwareRenderViewToTexture: public IRenderViewToTexture {
public:
    SoftwareRenderViewToTexture(SoftwareRenderer& renderer, APixelFormat format): mRenderer(renderer), mFormat(format) {}

    bool begin(IRenderer& renderer, glm::ivec2 surfaceSize, InvalidArea& invalidArea) override {
        if (!mTexture || mTexture->getImage().size() != glm::u32vec2(surfaceSize)) {
            mTexture = _cast<SoftwareTexture>(mRenderer.createTexture(surfaceSize, mFormat));
            invalidArea = InvalidArea::Full{};
        }

        mRenderer.mRenderTarget = const_cast<AImage*>(&mTexture->getImage());

        if (invalidArea.full()) {
            std::memset(mRenderer.mRenderTarget->modifiableBuffer().data(), 0, mRenderer.mRenderTarget->modifiableBuffer().getSize());
        }

        return true;
    }

    void end(IRenderer& renderer) override {
        mRenderer.mRenderTarget = nullptr;
    }

    void draw(ACanvas& canvas) override {
        canvas.rectangle(APaint{ATexturedBrush{mTexture}}, {0, 0}, mTexture->getSize());
    }

    [[nodiscard]]
    _<ITexture> getTexture() const override {
        return mTexture;
    }

private:
    SoftwareRenderer& mRenderer;
    _<SoftwareTexture> mTexture;
    APixelFormat mFormat;
};
