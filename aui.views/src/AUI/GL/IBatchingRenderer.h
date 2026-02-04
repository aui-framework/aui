// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "AUI/Render/IRenderer.h"

class IBatchingRenderer : public IRenderer {
public:
    ~IBatchingRenderer() override = default;
    void rectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size) override;
    void roundedRectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius) override;
    void rectangleBorder(const ABrush& brush, glm::vec2 position, glm::vec2 size, float lineWidth) override;
    void roundedRectangleBorder(
        const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius, int borderWidth) override;
    void boxShadow(glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) override;
    void boxShadowInner(
        glm::vec2 position, glm::vec2 size, float blurRadius, float spreadRadius, float borderRadius,
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
    void setWindow(AWindowBase* window) override;
    void flush();

protected:
    struct CmdRectangle {
        ABrush brush;
        glm::vec2 position;
        glm::vec2 size;
    };
    struct CmdRoundedRectangle {
        ABrush brush;
        glm::vec2 position;
        glm::vec2 size;
        float radius;
    };
    struct CmdRectangleBorder {
        ABrush brush;
        glm::vec2 position;
        glm::vec2 size;
        float lineWidth;
    };
    struct CmdRoundedRectangleBorder {
        ABrush brush;
        glm::vec2 position;
        glm::vec2 size;
        float radius;
        int borderWidth;
    };
    struct CmdBoxShadow {
        glm::vec2 position;
        glm::vec2 size;
        float blurRadius;
        AColor color;
    };
    struct CmdBoxShadowInner {
        glm::vec2 position;
        glm::vec2 size;
        float blurRadius;
        float spreadRadius;
        float borderRadius;
        AColor color;
        glm::vec2 offset;
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
    struct CmdPushMask {};
    struct CmdPopMask {};
    struct CmdSetBlending {
        Blending blending;
    };
    struct CmdNewRenderViewToTexture {};
    struct CmdSetWindow {
        AWindowBase* window;
    };
    using Cmd = std::variant<CmdRectangle, CmdRoundedRectangle, CmdRectangleBorder, CmdRoundedRectangleBorder,
                             CmdBoxShadow, CmdBoxShadowInner, CmdString, CmdLines, CmdPoints, CmdLinesPairs,
                             CmdSquareSector, CmdPushMask, CmdPopMask, CmdSetBlending, CmdNewRenderViewToTexture, CmdSetWindow>;

    virtual void handleCmds(std::vector<Cmd> cmds) = 0;

private:
    std::vector<Cmd> mCmds;
};
