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

//
// Created by Alex2772 on 12/7/2021.
//

#include "AFakeWindowInitializer.h"
#include "ASurface.h"
#include <AUI/Render/ADisplayList.h>
#include <AUI/Render/ADisplayListCanvas.hpp>
#include <AUI/Render/CanvasRenderer.h>
#include <AUI/Render/IRendererBackend.h>


struct AFakeWindowInitializer::Data {
    class FakeBackend: public IRendererBackend {
    public:
        _<ITexture> getNewTexture() override { return nullptr; }
        _unique<ITexture> createNewTexture() override { return nullptr; }
        _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override { return nullptr; }
        void rectangle(const ADisplayList::Rectangle& v, const APaint& paint) override {}
        void roundedRectangle(const ADisplayList::RoundedRectangle& v, const APaint& paint) override {}
        void rectangleBorder(const ADisplayList::RectangleBorder& v, const APaint& paint) override {}
        void roundedRectangleBorder(const ADisplayList::RoundedRectangleBorder& v, const APaint& paint) override {}
        void boxShadow(const ADisplayList::BoxShadow& v, const APaint& paint) override {}
        void boxShadowInner(const ADisplayList::BoxShadowInner& v, const APaint& paint) override {}
        void string(const ADisplayList::Text& v, const APaint& paint) override {}
        _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override { return nullptr; }
        void lines(const ADisplayList::Lines& v, const APaint& paint) override {}
        void points(const ADisplayList::Points& v, const APaint& paint) override {}
        void lines(const ADisplayList::LineBatches& v, const APaint& paint) override {}
        void squareSector(const ADisplayList::SquareSector& v, const APaint& paint) override {}
        void setTransformForced(const glm::mat4& transform) override { mTransform = transform; }
        const glm::mat4& getTransform() const override { return mTransform; }
        void setColorForced(const AColor& color) override { mColor = color; }
        const AColor& getColor() const override { return mColor; }
        void setBlending(Blending blending) override {}
        void pushMaskBefore() override {}
        void pushMaskAfter() override {}
        void popMaskBefore() override {}
        void popMaskAfter() override {}
        _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override { return nullptr; }
        void setWindow(ASurface* window) override { mWindow = window; }
        ASurface* getWindow() const noexcept override { return mWindow; }
        glm::mat4 getProjectionMatrix() const override { return glm::mat4(1.0f); }
        std::uint8_t getStencilDepth() const noexcept override { return mStencilDepth; }
        void setStencilDepth(std::uint8_t stencilDepth) override { mStencilDepth = stencilDepth; }
        float getRenderScale() const noexcept override { return 1.0f; }
        void setRenderScale(float renderScale) override {}
        void setAllowRenderToTexture(bool allow) override {}
        bool allowRenderToTexture() const noexcept override { return true; }
        void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override {}

    private:
        glm::mat4 mTransform = glm::mat4(1.0f);
        AColor mColor = AColor::WHITE;
        ASurface* mWindow = nullptr;
        std::uint8_t mStencilDepth = 0;
    };

    FakeBackend backend;
    ADisplayList displayList;
    ADisplayListCanvas canvas;
    CanvasRenderer renderer;

    Data(): canvas(displayList, backend), renderer(canvas) {}
};

void AFakeWindowInitializer::init(const IRenderingContext::Init& init) {
    mData = std::make_unique<Data>();
}

void AFakeWindowInitializer::destroyNativeWindow(ASurface& window) {

}

void AFakeWindowInitializer::beginPaint(ASurface& window) {

}

void AFakeWindowInitializer::endPaint(ASurface& window) {

}

void AFakeWindowInitializer::beginResize(ASurface& window) {

}

void AFakeWindowInitializer::endResize(ASurface& window) {

}

IRenderer& AFakeWindowInitializer::renderer() {
    return mData->renderer;
}

IRendererBackend& AFakeWindowInitializer::backend() {
    return mData->backend;
}

ACanvas& AFakeWindowInitializer::canvas() {
    return mData->canvas;
}
