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
// Created by Nelonn on 5/20/2026.
//

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <span>
#include <AUI/Common/AColor.h>
#include <AUI/Render/ABrush.h>
#include <AUI/Render/ABorderStyle.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Font/AFontStyle.h>
#include <AUI/Geometry2D/ARect.h>
#include <AUI/Util/AAngleRadians.h>
#include <AUI/Util/AArrayView.h>
#include <AUI/Common/AString.h>
#include <AUI/ASS/Property/Backdrop.h>
#include <AUI/Render/APaint.hpp>
#include <AUI/Render/IRenderer.h>

#include <AUI/Render/IRendererBackend.h>

class ACanvas {
public:

    virtual ~ACanvas() = default;

    virtual IRendererBackend& renderer() = 0;

    virtual size_t save() {
        size_t res = mStates.size();
        mStates.push_back(State{mTransform, mBaseTransform, mMaskStackDepth, mRenderTargetStackDepth, mLayerStackDepth});
        return res;
    }

    virtual void restore() {
        if (!mStates.empty()) {
            State s = mStates.back();
            mStates.pop_back();
            mTransform = s.transform;
            mBaseTransform = s.baseTransform;
            while (mMaskStackDepth > s.maskStackDepth) popMask();
            while (mRenderTargetStackDepth > s.renderTargetStackDepth) popRenderTarget();
            while (mLayerStackDepth > s.layerStackDepth) popLayer();
        }
    }

    virtual void restore(size_t targetStackSize) {
        while (mStates.size() > targetStackSize) {
            restore();
        }
    }

    void pushMask(_<ITexture> mask, const glm::vec4& maskRect) {
        mMaskStackDepth++;
        pushMaskImpl(std::move(mask), maskRect);
    }
    void popMask() {
        AUI_ASSERT(mMaskStackDepth > 0);
        mMaskStackDepth--;
        popMaskImpl();
    }

    void pushRenderTarget(_<ITexture> texture) {
        mRenderTargetStackDepth++;
        pushRenderTargetImpl(std::move(texture));
    }
    void popRenderTarget() {
        AUI_ASSERT(mRenderTargetStackDepth > 0);
        mRenderTargetStackDepth--;
        popRenderTargetImpl();
    }

    virtual void clear() = 0;

    void pushLayer() {
        mLayerStackDepth++;
        pushLayerImpl();
    }
    void popLayer() {
        AUI_ASSERT(mLayerStackDepth > 0);
        mLayerStackDepth--;
        popLayerImpl();
    }

    virtual void pushMaskImpl(_<ITexture> mask, const glm::vec4& maskRect) = 0;
    virtual void popMaskImpl() = 0;
    virtual void pushRenderTargetImpl(_<ITexture> texture) = 0;
    virtual void popRenderTargetImpl() = 0;
    virtual void pushLayerImpl() = 0;
    virtual void popLayerImpl() = 0;

    static bool isSimple(const glm::mat4& m) noexcept {
        return glm::abs(m[0][1]) < 0.0001f && glm::abs(m[0][2]) < 0.0001f && glm::abs(m[0][3]) < 0.0001f &&
               glm::abs(m[1][0]) < 0.0001f && glm::abs(m[1][2]) < 0.0001f && glm::abs(m[1][3]) < 0.0001f &&
               glm::abs(m[2][0]) < 0.0001f && glm::abs(m[2][1]) < 0.0001f && glm::abs(m[2][3]) < 0.0001f;
    }

    virtual void setTransform(const glm::mat4& transform) {
        if (isSimple(transform)) {
            mTransform = mTransform * transform;
        } else {
            mBaseTransform = mBaseTransform * mTransform * transform;
            mTransform = glm::mat4(1.0f);
        }
    }

    virtual void rectangle(const APaint& paint,
                           glm::vec2 position,
                           glm::vec2 size) = 0;

    virtual void roundedRectangle(const APaint& paint,
                                  glm::vec2 position,
                                  glm::vec2 size,
                                  float radius) = 0;

    virtual void rectangleBorder(const APaint& paint,
                                 glm::vec2 position,
                                 glm::vec2 size,
                                 float lineWidth = 1.f) = 0;

    virtual void roundedRectangleBorder(const APaint& paint,
                                        glm::vec2 position,
                                        glm::vec2 size,
                                        float radius,
                                        int borderWidth) = 0;

    virtual void boxShadow(const APaint& paint,
                           glm::vec2 position,
                           glm::vec2 size,
                           float blurRadius,
                           const AColor& color) = 0;

    virtual void boxShadowInner(const APaint& paint,
                                glm::vec2 position,
                                glm::vec2 size,
                                float blurRadius,
                                float spreadRadius,
                                float borderRadius,
                                const AColor& color,
                                glm::vec2 offset) = 0;

    virtual void string(const APaint& paint,
                        glm::vec2 position,
                        const AString& string,
                        const AFontStyle& fs = {}) = 0;

    virtual void prerenderedString(const APaint& paint,
                                   glm::vec2 position,
                                   const _<IRenderer::IPrerenderedString>& prerenderedString) = 0;

    virtual void glyphRect(const _<ITexture>& texture, glm::vec2 position, glm::vec2 size, glm::vec2 u1, glm::vec2 u2, const AColor& color, bool isSubpixel) = 0;

    virtual void lines(const APaint& paint, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) = 0;

    void lines(const APaint& paint, AArrayView<glm::vec2> points, const ABorderStyle& style = ABorderStyle::Solid{}) {
        lines(paint, points, style, 1_dp);
    }

    virtual void points(const APaint& paint, AArrayView<glm::vec2> points, AMetric size) = 0;

    virtual void lines(const APaint& paint, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) = 0;

    void lines(const APaint& paint, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style = ABorderStyle::Solid{}) {
        lines(paint, points, style, 1_dp);
    }

    virtual void squareSector(const APaint& paint,
                              const glm::vec2& position,
                              const glm::vec2& size,
                              AAngleRadians begin,
                              AAngleRadians end) = 0;

    virtual void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Any> backdrops) = 0;

    float getRenderScale() const noexcept { return mRenderScale; }
    void setRenderScale(float renderScale) noexcept { mRenderScale = renderScale; }
    virtual _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) = 0;
    virtual _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;

    const glm::mat4 getTransform() const noexcept { return mBaseTransform * mTransform; }
    virtual void setTransformForced(const glm::mat4& transform) noexcept {
        if (isSimple(transform)) {
            mBaseTransform = glm::mat4(1.0f);
            mTransform = transform;
        } else {
            mBaseTransform = transform;
            mTransform = glm::mat4(1.0f);
        }
    }

    void translate(const glm::vec2& offset) {
        mTransform = glm::translate(mTransform, glm::vec3(offset, 0.f));
    }
    void rotate(const glm::vec3& axis, AAngleRadians angle) {
        mBaseTransform = mBaseTransform * mTransform * glm::rotate(glm::mat4(1.f), angle.radians(), axis);
        mTransform = glm::mat4(1.f);
    }
    void rotate(AAngleRadians angle) {
        rotate({0.f, 0.f, 1.f}, angle);
    }

    const glm::mat4& getBaseTransform() const noexcept { return mBaseTransform; }
    void setBaseTransform(const glm::mat4& transform) noexcept { mBaseTransform = transform; }

protected:
    struct State {
        glm::mat4 transform;
        glm::mat4 baseTransform;
        size_t maskStackDepth;
        size_t renderTargetStackDepth;
        size_t layerStackDepth;
    };
    std::vector<State> mStates;
    glm::mat4 mTransform = glm::mat4(1.0f);
    glm::mat4 mBaseTransform = glm::mat4(1.0f);
    float mRenderScale = 1.0f;
    size_t mMaskStackDepth = 0;
    size_t mRenderTargetStackDepth = 0;
    size_t mLayerStackDepth = 0;
};
