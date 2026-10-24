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

#include <glm/glm.hpp>
#include <vector>
#include <span>
#include <functional>
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
#include <AUI/Render/IRendererText.hpp>
#include <AUI/Render/APaint.hpp>
#include <AUI/Render/AClipOp.hpp>

class ACanvas {
public:

    virtual ~ACanvas() = default;

    virtual size_t save() = 0;
    virtual void restore() = 0;
    virtual void restore(size_t targetStackSize) = 0;

    virtual void pushClipRect(const ARect<float>& rect, AClipOp op = AClipOp::OP_INTERSECT) = 0;
    virtual void pushClipRoundedRect(const ARect<float>& rect, float radius, AClipOp op = AClipOp::OP_INTERSECT) = 0;
    virtual void popClipRect() = 0;

    virtual void pushMask(const _<ITexture>& maskTexture, const glm::vec4& maskRect) = 0;
    virtual void popMask() = 0;

    virtual void clear(const AColor& color = AColor::TRANSPARENT_BLACK) = 0;

    virtual void pushLayer() = 0;
    virtual void popLayer() = 0;

    static bool isSimple(const glm::mat4& m) noexcept {
        return m[0][0] > 0.0001f && glm::abs(m[0][1]) < 0.0001f && glm::abs(m[0][2]) < 0.0001f && glm::abs(m[0][3]) < 0.0001f &&
               glm::abs(m[1][0]) < 0.0001f && m[1][1] > 0.0001f && glm::abs(m[1][2]) < 0.0001f && glm::abs(m[1][3]) < 0.0001f &&
               glm::abs(m[2][0]) < 0.0001f && glm::abs(m[2][1]) < 0.0001f && m[2][2] > 0.0001f && glm::abs(m[2][3]) < 0.0001f;
    }

    virtual void setTransform(const glm::mat4& transform) = 0;

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
                                   const _<aui::IPrerenderedString>& prerenderedString) = 0;

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

    /**
     * @brief Escape hatch to issue raw OpenGL calls from within AUI's rendering pipeline.
     * @param callback invoked with the canvas's current transform when the enqueued command is actually
     *        executed.
     * @details
     * @warning Think twice before reaching for this. AUI batches, reorders and culls draw commands for
     * performance; everything you draw through `rectangle()`, `string()`, etc. plays nicely with that.
     * A raw GL callback does not: it is an opaque, unmovable barrier in the draw list (it can't be
     * reordered, merged or culled), so using it anywhere other than rarely will quietly undo the
     * optimizations the rest of your UI relies on, and it ties your view to the OpenGL backend
     * specifically - it is silently skipped (with a logged warning) on any other [IRendererBackend],
     * such as the software rasterizer. If you can express what you want with the regular `ACanvas` API,
     * [ITexture] + a custom shader, or by rendering to an offscreen texture and drawing that texture with
     * `rectangle()`, do that instead.
     *
     * If you still need this - for a one-off effect that genuinely can't be expressed otherwise - keep in
     * mind:
     * - The callback runs synchronously, later, when this part of the draw list is actually rendered - not
     *   when you call `enqueueGlCallback`. Don't capture anything by reference that might be gone by then.
     * - The clip rect active at enqueue time is applied as the GL scissor box beforehand, same as for any
     *   other draw command, so you don't need to (and can't easily) handle clipping yourself.
     * - AUI makes no promise about what GL state (bound program, VAO, textures, blend function, depth/
     *   stencil state, etc.) is active when your callback runs, and it makes no promise about restoring
     *   its own state after your callback either. You must save/restore (or at least not rely on and not
     *   leak) whatever GL state you touch - see `examples/ui/opengl_simple` for the save/cleanup pattern.
     * - `enqueueGlCallback` is only ever meaningful together with the OpenGL backend
     *   ([OpenGLBackend], under `AUI/Render/ARender/GL`). It is the one sanctioned crack in that
     *   boundary - everything else in the framework is written against [IRenderer]/[ACanvas] and does not
     *   know OpenGL exists.
     */
    virtual void enqueueGlCallback(std::function<void(const glm::mat4& transform)> callback) = 0;

    virtual _<aui::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) = 0;
    virtual _<aui::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;

    virtual const glm::mat4 getTransform() const noexcept = 0;
    virtual void setTransformForced(const glm::mat4& transform) noexcept = 0;

    virtual void translate(const glm::vec2& offset) = 0;
    virtual void rotate(const glm::vec3& axis, AAngleRadians angle) = 0;
    virtual void rotate(AAngleRadians angle) = 0;

    virtual void scale(const glm::vec2& multiplier) = 0;
    void scale(float multiplier) {
        scale({multiplier, multiplier});
    }

    virtual const glm::mat4& getBaseTransform() const noexcept = 0;
    virtual void setBaseTransform(const glm::mat4& transform) noexcept = 0;
};
