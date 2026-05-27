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

#include <AUI/Render/IRendererBackend.h>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/RendererCanvas.h>
#include <AUI/Render/ADisplayList.h>
#include <AUI/Render/ADisplayListCanvas.hpp>
#include <AUI/Render/FontAtlas.hpp>

namespace {
    class FakeBackend: public IRendererBackend {
    public:
        FakeBackend() = default;

        void solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, const APaint& paint) override {}
        void gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, const APaint& paint) override {}
        void texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, const APaint& paint) override {}
        void solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) override {}
        void gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) override {}
        void texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) override {}
        void rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, const APaint& paint) override {}
        void roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, const APaint& paint) override {}
        void boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, const APaint& paint) override {}
        void boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, const APaint& paint) override {}
        void glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, const APaint& paint) override {}
        void lines(const ADisplayList::Lines& v, const glm::mat4& transform, const APaint& paint) override {}
        void points(const ADisplayList::Points& v, const glm::mat4& transform, const APaint& paint) override {}
        void lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, const APaint& paint) override {}
        void squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, const APaint& paint) override {}
        void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override {}

        _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override {
            auto entryData = aui::getFontEntryData(*this, getFontEntryDataCache(), style);
            return _new<aui::MultiStringCanvas>(*this, entryData, getCharacterDataCache(), style);
        }
        _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override {
            auto c = newMultiStringCanvas(fs);
            c->addString(position, text);
            return c->finalize();
        }

        _<ITexture> createTexture(glm::u32vec2 size, APixelFormat format) override { return nullptr; }
        float getRenderScale() const noexcept override { return 1.0f; }
        void setRenderScale(float renderScale) override {}
        void setAllowRenderToTexture(bool allow) override {}
        bool allowRenderToTexture() const noexcept override { return true; }
        _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override { return nullptr; }
        void setWindow(ASurface* window) override {}
        ASurface* getWindow() const noexcept override { return nullptr; }
        glm::mat4 getProjectionMatrix() const override { return glm::mat4(1.0f); }

        ADeque<aui::FontAtlas>& getFontEntryDataCache() override { return mFontEntryData; }
        ADeque<aui::CharacterData>& getCharacterDataCache() override { return mCharData; }

    private:
        ADeque<aui::FontAtlas> mFontEntryData;
        ADeque<aui::CharacterData> mCharData;
    };
}

struct AFakeWindowInitializer {
    FakeBackend backend;
    ADisplayList dl;
    ADisplayListCanvas canvas;
    RendererCanvas renderer;
    AFakeWindowInitializer(): canvas(dl, backend), renderer(canvas) {}
};
