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

namespace {

class StubRenderer : public IRendererBackend {
public:
    class StubPrerenderedString : public IRenderer::IPrerenderedString {
    public:
        void draw() override {}
        ~StubPrerenderedString() override = default;
        int getWidth() override { return 1; }
        int getHeight() override { return 1; }
    };

    StubRenderer() = default;
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
    void rectangle(const ADisplayList::Rectangle& v, const APaint& paint) override {}
    void roundedRectangle(const ADisplayList::RoundedRectangle& v, const APaint& paint) override {}
    void rectangleBorder(const ADisplayList::RectangleBorder& v, const APaint& paint) override {}
    void roundedRectangleBorder(
        const ADisplayList::RoundedRectangleBorder& v, const APaint& paint) override {}
    void boxShadow(const ADisplayList::BoxShadow& v, const APaint& paint) override {}
    void boxShadowInner(
        const ADisplayList::BoxShadowInner& v, const APaint& paint) override {}
    void string(const ADisplayList::Text& v, const APaint& paint) override {}
    _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override {
        return _new<StubPrerenderedString>();
    }
    void lines(const ADisplayList::Lines& v, const APaint& paint) override {}
    void points(const ADisplayList::Points& v, const APaint& paint) override {}
    void lines(const ADisplayList::LineBatches& v, const APaint& paint) override {}
    void squareSector(
        const ADisplayList::SquareSector& v, const APaint& paint) override {}
    void pushMaskBefore() override {}
    void pushMaskAfter() override {}
    void popMaskBefore() override {}
    void popMaskAfter() override {}
    void setBlending(Blending blending) override {}
    [[nodiscard]] _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override { return nullptr; }
    void setWindow(ASurface* window) override {}
    glm::mat4 getProjectionMatrix() const override { return glm::mat4(1.0f); }

    _<ITexture> getNewTexture() override { return createNewTexture(); }
    float getRenderScale() const noexcept override { return 1.0f; }
    void setRenderScale(float renderScale) override {}
    void setAllowRenderToTexture(bool allow) override {}
    bool allowRenderToTexture() const noexcept override { return true; }
    void setTransformForced(const glm::mat4& transform) override {}
    void setColorForced(const AColor& color) override {}
    void backdrops(const ADisplayList::Backdrop& v, const APaint& paint) override {}
    void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override {}

    _unique<ITexture> createNewTexture() override {
        class Stub : public ITexture {
        public:
            void setImage(AImageView image) override {}
            ~Stub() override = default;
        };
        return std::make_unique<Stub>();
    }
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
