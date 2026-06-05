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

#include <benchmark/benchmark.h>
#include "UIBenchmarkScene.h"
#include "AUI/UITest.h"
#include "AUI/Util/AStubWindowManager.h"
#include <AUI/Render/FontAtlas.hpp>
#include <AUI/Platform/AFontManager.h>

namespace {

class StubRenderer : public IRendererBackend {
public:
    class StubPrerenderedString : public IRenderer::IPrerenderedString {
    public:
        void draw(ACanvas& canvas) override {}
        ~StubPrerenderedString() override = default;
        int getWidth() override { return 1; }
        int getHeight() override { return 1; }
    };

    StubRenderer() : mFontCache(AFontManager::inst().createCache(this)) {}
    ~StubRenderer() override = default;
    _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override {
        class Stub : public IRenderer::IMultiStringCanvas {
        public:
            ~Stub() override = default;
            void addString(const glm::ivec2& position, AStringView text) noexcept override {}
            void addString(const glm::ivec2& position, std::u32string_view text) noexcept override {}
            _<IRenderer::IPrerenderedString> finalize() noexcept override { return _new<StubPrerenderedString>(); }
        };
        return _new<Stub>();
    }
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
    _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override {
        return _new<StubPrerenderedString>();
    }
    void lines(const ADisplayList::Lines& v, const glm::mat4& transform, const APaint& paint) override {}
    void points(const ADisplayList::Points& v, const glm::mat4& transform, const APaint& paint) override {}
    void lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, const APaint& paint) override {}
    void squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, const APaint& paint) override {}

    glm::mat4 getProjectionMatrix() const override { return glm::mat4(1.0f); }

    class Stub : public ITexture {
    public:
        void upload(AImageView image) override { mSize = image.size(); }
        [[nodiscard]] glm::u32vec2 getSize() const override { return mSize; }
        [[nodiscard]] APixelFormat getFormat() const override { return APixelFormat::R8G8B8A8_UNORM; }
        ~Stub() override = default;
    private:
        glm::u32vec2 mSize = { 0, 0 };
    };

    _<ITexture> createTexture(glm::u32vec2 size, APixelFormat format, TextureFilter filter) override {
        auto t = _new<Stub>();
        return t;
    }
    void setMask(const _<ITexture>& mask, const glm::vec4& maskRect) override {}
    AMergedMask mergeMasks(const _<ITexture>& mask1, const glm::vec4& mask1Rect,
                           const _<ITexture>& mask2, const glm::vec4& mask2Rect) override {
        return { nullptr, glm::vec4(0.f) };
    }
    _<ITexture> createRectMask(const ARect<float>& rect, bool inverted, const ARect<float>& bounds) override {
        return nullptr;
    }
    void setRenderTarget(const _<ITexture>& texture, glm::uvec2 size) override {}
    void setClipRect(const ARect<float>& rect) override {}
    void setRenderMaskMode(bool enabled) override {}
    void clear(const AColor& color) override {}
    void beginRenderPass(const _<ITexture>& target) override {}
    void endRenderPass() override {}
    void flush() override {}
    _unique<IOffscreenRenderPass> beginOffscreen(const _<ITexture>& renderTarget) override { return nullptr; }
    void endOffscreen(_unique<IOffscreenRenderPass> pass) override {}
    void setAllowRenderToTexture(bool allow) override {}
    bool allowRenderToTexture() const noexcept override { return true; }
    void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override {}
    const _<aui::AFontCache>& getFontCache() override { return mFontCache; }


private:
    _<aui::AFontCache> mFontCache;
};

}   // namespace

static void UIStyleLegacy(benchmark::State& state) {
    // https://github.com/aui-framework/aui/issues/572
    //
    // This benchmark implements old animation approach based on manually calling setCustomStyle each frame.
    //
    // What does this benchmark tests:
    // - performance of ASS by updating style each frame
    // - side effects caused by updating styles (i.e., whether layout engine triggers recomposition when it does not
    //   de facto needed)
    //
    // What does NOT this benchmark test:
    // - renderer performance
    // - accuracy of animation (i.e., we don't measure frame time to achieve perfect timing)
    //

    uitest::setup();

    AStubWindowManager::setConfig({
      .renderer = std::make_unique<StubRenderer>(),
    });

    auto window = _new<AWindow>();
    _<AView> button = declarative::Button {};
    window->setContents(declarative::Centered { button });
    window->pack();
    window->show();

    uitest::frame();

    float time = 0;
    for (auto _2 : state) {
        button->setCustomStyle({ ass::BackgroundSolid {
          glm::mix(glm::vec4(AColor::RED), glm::vec4(AColor::BLUE), glm::sin(time += 0.01f)) } });
        uitest::frame();
    }
}

BENCHMARK(UIStyleLegacy);
