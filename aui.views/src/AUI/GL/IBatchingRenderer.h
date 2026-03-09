// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include "AUI/Render/IRenderer.h"

class IBatchingRenderer : public IRenderer {
public:
    struct CmdRectangle {
        ABrush brush;
        glm::vec2 position;
        glm::vec2 size;
        zIndex_t zIndex;
    };
    struct CmdRoundedRectangle {
        ABrush brush;
        glm::vec2 position;
        glm::vec2 size;
        float radius;
        zIndex_t zIndex;
    };
    struct CmdRectangleBorder {
        ABrush brush;
        glm::vec2 position;
        glm::vec2 size;
        float lineWidth;
        zIndex_t zIndex;
    };
    struct CmdRoundedRectangleBorder {
        ABrush brush;
        glm::vec2 position;
        glm::vec2 size;
        float radius;
        int borderWidth;
        zIndex_t zIndex;
    };
    struct CmdBoxShadow {
        glm::vec2 position;
        glm::vec2 size;
        float blurRadius;
        AColor color;
        zIndex_t zIndex;
    };
    struct CmdBoxShadowInner {
        glm::vec2 position;
        glm::vec2 size;
        float blurRadius;
        float spreadRadius;
        float borderRadius;
        AColor color;
        glm::vec2 offset;
        zIndex_t zIndex;
    };
    struct CmdString {
        glm::vec2 position;
        AString string;
        AFontStyle fs;
    };
    struct CmdLines {
        ABrush brush;
        AArrayView<glm::vec2> points;
        ABorderStyle style;
        AMetric width;
    };
    struct CmdPoints {
        ABrush brush;
        AArrayView<glm::vec2> points;
        AMetric size;
    };
    struct CmdLinesPairs {
        ABrush brush;
        AArrayView<std::pair<glm::vec2, glm::vec2>> points;
        ABorderStyle style;
        AMetric width;
    };
    struct CmdSquareSector {
        ABrush brush;
        glm::vec2 position;
        glm::vec2 size;
        AAngleRadians begin;
        AAngleRadians end;
    };
    struct CmdNewRenderViewToTexture {};
    struct CmdSetWindow {
        AWindowBase* window;
    };
    using BatchId_t = uint32_t;
    union BatchId {
        BatchId_t value;
        struct {
            unsigned char brushId : 8;
            unsigned char cmdId : 8;
            zIndex_t zIndex : 16;
        };
        // TODO: finish bit order fix
        // static BatchId new(int16_t z, uint8_t cmd, uint8_t brush) {
        //     return { static_cast<uint32_t>(z << 16) | static_cast<uint32_t>(cmd << 8) | static_cast<uint32_t>(brush) };
        // }
        // auto operator<=>(const BatchId&) const = default;
    };
    struct Cmd {
        glm::mat4 transform;
        AColor color;
        using Arg = std::variant<
            CmdRectangle, CmdRoundedRectangle, CmdRectangleBorder, CmdRoundedRectangleBorder, CmdBoxShadow,
            CmdBoxShadowInner, CmdString, CmdLines, CmdPoints, CmdLinesPairs, CmdSquareSector,
            CmdNewRenderViewToTexture, CmdSetWindow>;
        Arg arg;
        BatchId batchId;
    };

    ~IBatchingRenderer() override = default;
    void rectangle(const ABrush& brush, glm::vec2 position, zIndex_t zIndex, glm::vec2 size) override;
    void roundedRectangle(const ABrush& brush, glm::vec2 position, zIndex_t zIndex, glm::vec2 size, float radius) override;
    void rectangleBorder(const ABrush& brush, glm::vec2 position, zIndex_t zIndex, glm::vec2 size, float lineWidth) override;
    void roundedRectangleBorder(
        const ABrush& brush, glm::vec2 position, zIndex_t zIndex, glm::vec2 size, float radius, int borderWidth) override;
    void boxShadow(glm::vec2 position, zIndex_t zIndex, glm::vec2 size, float blurRadius, const AColor& color) override;
    void boxShadowInner(
        glm::vec2 position, zIndex_t zIndex, glm::vec2 size, float blurRadius, float spreadRadius, float borderRadius,
        const AColor& color, glm::vec2 offset) override;
    void string(glm::vec2 position, const AString& string, const AFontStyle& fs) override;
    void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) override;
    void points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) override;
    void
    lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style,
          AMetric width) override;
    void squareSector(
        const ABrush& brush, const glm::vec2& position, const glm::vec2& size, AAngleRadians begin,
        AAngleRadians end) override;
    void pushMaskBefore() override;
    void pushMaskAfter() override;
    void popMaskBefore() override;
    void popMaskAfter() override;
    void setBlending(Blending blending) override;
    void flush();

protected:
    void enqueueCommand(Cmd::Arg arg) {
        mCmds.emplace_back(Cmd{
            .transform = getTransform(),
            .color = getColor(),
            .arg = std::move(arg),
        });
    }

    virtual void handleCmds(std::vector<Cmd> cmds) = 0;

private:
    std::vector<Cmd> mCmds;
};
