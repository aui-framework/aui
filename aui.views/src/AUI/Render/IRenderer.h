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
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Common/ASide.h>
#include <AUI/Common/AColor.h>
#include <AUI/Render/APaint.hpp>
#include <AUI/Util/AArrayView.h>
#include <AUI/Render/ABorderStyle.h>
#include <AUI/Util/AMetric.h>
#include "ATextLayoutHelper.h"
#include <AUI/Font/FontRendering.hpp>
#include <AUI/Render/IRendererText.hpp>
#include <AUI/ASS/Property/Backdrop.h>
#include <AUI/Image/APixelFormat.h>

class AColor;
class ASurface;
class AFontStyle;
class ACanvas;

/**
 * @brief Base class for renderer.
 * @ingroup views
 * @details
 * This is the only seam the rest of the framework (views, layouts, ASS, ...) is written against; nothing
 * outside `AUI/Render/ARender` knows or cares how an `IRenderer` actually draws anything.
 *
 * AUI's own implementation - [AUIRenderer], batching through `ADrawList` into an `IRendererBackend`
 * (OpenGL or software) - lives entirely under `AUI/Render/ARender`. That whole folder is one
 * self-contained, deletable unit: a different renderer (e.g. backed by Skia) is a new class that
 * implements `IRenderer` (and `ACanvas`) directly and knows nothing about `IRendererBackend`,
 * `ADrawList`, or OpenGL - it is not a plugin *for* `AUIRenderer`, it *replaces* it.
 */
class IRenderer: public aui::noncopyable {
public:
    using IPrerenderedString = aui::IPrerenderedString;
    using IMultiStringCanvas = aui::IMultiStringCanvas;

public:
    IRenderer() = default;
    virtual ~IRenderer() = default;

    /**
     * @brief Creates new canvas for batching multiple <code>prerender</code> string calls.
     * @return a new instance of <code>IMultiStringCanvas</code>
     */
    virtual _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) = 0;

    /**
     * @brief Analyzes string and creates an instance of <code>IRenderer::IPrerenderedString</code>.
     */
    virtual _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;
 
    virtual void setColorForced(const AColor& color) = 0;

    virtual const AColor& getColor() const = 0;

    virtual void setTransform(const glm::mat4& transform) = 0;

    virtual void setTransformForced(const glm::mat4& transform) = 0;

    virtual void setBlending(Blending blending) = 0;

    virtual glm::mat4 getProjectionMatrix() const = 0;

    virtual glm::mat4 getTransform() = 0;

    virtual void translate(const glm::vec2& offset) = 0;

    virtual void rotate(const glm::vec3& axis, AAngleRadians angle) = 0;

    virtual void rotate(AAngleRadians angle) = 0;

    virtual void setAllowRenderToTexture(bool allowRenderToTexture) = 0;

    [[nodiscard]]
    virtual bool allowRenderToTexture() const noexcept = 0;

    virtual void setRenderScale(float render_scale) = 0;

    virtual float getRenderScale() const noexcept = 0;

    virtual void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Any> backdrops) = 0;

    virtual void stub(glm::vec2 position, glm::vec2 size) = 0;

    virtual ACanvas& canvas() = 0;
};
