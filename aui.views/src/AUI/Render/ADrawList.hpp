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

#include <variant>
#include <AUI/Render/ABorderStyle.h>
#include <AUI/ASS/Property/Backdrop.h>
#include <AUI/Font/AFontStyle.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Common/AVector.h>
#include <AUI/Render/ABrush.h>
#include <AUI/Render/APaint.hpp>
#include <glm/glm.hpp>
#include <AUI/Geometry2D/ARect.h>
#include <AUI/Render/ITexture.h>
#include <AUI/Render/IRendererText.hpp>
#include <AUI/Render/AClipOp.hpp>

class IRenderer;
class IRendererBackend;

class ADrawList {
public:
    struct RectInstance {
        glm::vec2 position;
        glm::vec2 size;
        AColor color;
    };
    struct SolidRectangles {
        AVector<RectInstance> instances;
    };
    struct GradientRectangles {
        AVector<RectInstance> instances;
        AStaticVector<ALinearGradientBrush::ColorEntry, 2> colors;
        AAngleRadians rotation;
    };
    struct TexturedRectangles {
        AVector<RectInstance> instances;
        _<ITexture> texture;
        glm::vec2 uv1 = { 0.f, 0.f };
        glm::vec2 uv2 = { 1.f, 1.f };
        bool premultiplied = false;
    };
    struct SolidRoundedRectangles {
        AVector<RectInstance> instances;
        float radius;
    };
    struct GradientRoundedRectangles {
        AVector<RectInstance> instances;
        float radius;
        AStaticVector<ALinearGradientBrush::ColorEntry, 2> colors;
        AAngleRadians rotation;
    };
    struct TexturedRoundedRectangles {
        AVector<RectInstance> instances;
        float radius;
        _<ITexture> texture;
        glm::vec2 uv1 = { 0.f, 0.f };
        glm::vec2 uv2 = { 1.f, 1.f };
        bool premultiplied = false;
    };
    struct RectangleBorders {
        AVector<RectInstance> instances;
        float lineWidth;
    };
    struct RoundedRectangleBorders {
        AVector<RectInstance> instances;
        float radius;
        int borderWidth;
    };
    struct BoxShadow {
        glm::vec2 position;
        glm::vec2 size;
        float blurRadius;
        AColor color;
    };
    struct BoxShadowInner {
        glm::vec2 position;
        glm::vec2 size;
        float blurRadius;
        float spreadRadius;
        float borderRadius;
        AColor color;
        glm::vec2 offset;
    };
    struct GlyphInstance {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec2 u1;
        glm::vec2 u2;
        AColor color;
    };
    struct Glyphs {
        AVector<GlyphInstance> instances;
        _<ITexture> texture;
        AColor color;
        bool isSubpixel;
    };
    struct Lines {
        AVector<glm::vec2> points;
        ABorderStyle style;
        AMetric width;
    };
    struct LineBatches {
        AVector<std::pair<glm::vec2, glm::vec2>> points;
        ABorderStyle style;
        AMetric width;
    };
    struct Points {
        AVector<glm::vec2> points;
        AMetric size;
    };
    struct SquareSector {
        glm::vec2 position;
        glm::vec2 size;
        AAngleRadians begin;
        AAngleRadians end;
    };
    struct Backdrop {
        glm::ivec2 position;
        glm::ivec2 size;
        AVector<ass::Backdrop::Any> backdrops;
    };

    struct PushClipRect {
        ARect<float> rect;
        AClipOp op;
    };
    struct PushClipRoundedRect {
        ARect<float> rect;
        float radius;
        AClipOp op;
    };
    struct PopClipRect {};
    struct Clear {
        AColor color;
    };
    struct PushLayer {};
    struct PopLayer {};
    struct PushMask {
        _<ITexture> mask;
        glm::vec4 maskRect;
    };
    struct PopMask {};

    struct StoredCommand {
        using Command = std::variant<
            SolidRectangles, GradientRectangles, TexturedRectangles, SolidRoundedRectangles, GradientRoundedRectangles,
            TexturedRoundedRectangles, RectangleBorders, RoundedRectangleBorders, BoxShadow, BoxShadowInner, Glyphs,
            Lines, LineBatches, Points, SquareSector, Backdrop, PushClipRect, PushClipRoundedRect, PopClipRect, Clear, PushLayer, PopLayer, PushMask, PopMask>;
        Command command;
        glm::mat4 transform;
        APaint paint;
    };

    struct LogicalMask {
        enum class Type {
            Texture,
            Rect,
            RoundedRect
        } type;

        AClipOp op = AClipOp::OP_INTERSECT;

        ARect<float> rect;
        float radius = 0.f;

        _<ITexture> texture;
        glm::vec4 maskRect;

        glm::mat4 transform;
    };

    struct Entity {
        StoredCommand::Command command;
        glm::mat4 transform;
        APaint paint;
        ARect<float> boundingBox;
        bool isObscured = false;
        ARect<float> clipRect;
        _<ITexture> mask;
        glm::vec4 maskRect;
        AVector<LogicalMask> activeMasks;
        bool culled = false;
    };

    struct RenderPass {
        _<ITexture> target;
        glm::uvec2 size;
        AVector<Entity> entities;
    };

    void add(StoredCommand::Command cmd, const glm::mat4& transform, APaint paint);

    void clear() {
        mCommands.clear();
        mOpaqueRects.clear();
        mEntities.clear();
        mPasses.clear();
    }

    void optimize();

    void draw(IRendererBackend& renderer, const _<ITexture>& windowTarget);

    void reorderAndBatch();
    void resolveEntities();
    void computeOverlaps();
    void resolveClips();
    void resolveLogicalMasks();
    void resolvePhysicalMasks(IRendererBackend& renderer);
    void resolvePasses(IRendererBackend& renderer, const _<ITexture>& windowTarget);

private:
    AVector<StoredCommand> mCommands;
    AVector<ARect<float>> mOpaqueRects;
    AVector<Entity> mEntities;
    AVector<RenderPass> mPasses;
};
