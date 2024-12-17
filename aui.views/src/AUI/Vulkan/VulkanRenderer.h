/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "AUI/Vulkan/LogicalDevice.h"
#include "AUI/Vulkan/CommandPool.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Vulkan/Instance.h"

class VulkanRenderer : public IRenderer {
    friend class VulkanPrerenderedString;
    friend class VulkanMultiStringCanvas;

public:
    struct FontEntryData : aui::noncopyable {
        Util::SimpleTexturePacker texturePacker;
        gl::Texture2D texture;
        bool isTextureInvalid = true;

        FontEntryData() { texture.bind(); }
    };

protected:
    _unique<ITexture> createNewTexture() override;

public:
    VulkanRenderer();
    ~VulkanRenderer() override = default;

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

    _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    void rectImpl(glm::vec2 position, glm::vec2 size);

    void setBlending(Blending blending) override;

    _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;

    glm::mat4 getProjectionMatrix() const override;

    void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) override;
    void points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) override;
    void
    lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style,
          AMetric width) override;
    _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override;

    void squareSector(
        const ABrush& brush, const glm::vec2& position, const glm::vec2& size, AAngleRadians begin,
        AAngleRadians end) override;

    void pushMaskBefore() override;
    void pushMaskAfter() override;
    void popMaskBefore() override;
    void popMaskAfter() override;

    void beginPaint(glm::uvec2 windowSize);
    void endPaint();

    void bindViewport() const noexcept;

    [[nodiscard]]
    uint32_t getSupersamplingRatio() const noexcept;

    [[nodiscard]]
    glm::uvec2 viewportSize() const noexcept {
        return mViewportSize;
    }

    [[nodiscard]]
    const aui::vk::Instance& instance() const noexcept {
        return mInstance;
    }

    [[nodiscard]]
    VkPhysicalDevice physicalDevice() const noexcept {
        return mPhysicalDevice;
    }

    [[nodiscard]]
    const aui::vk::LogicalDevice& logicalDevice() const noexcept {
        return mLogicalDevice;
    }

    [[nodiscard]]
    const aui::vk::CommandPool& commandPool() const noexcept {
        return mCommandPool;
    }

private:
    glm::uvec2 mViewportSize;
    aui::vk::Instance mInstance;
    VkPhysicalDevice mPhysicalDevice;
    aui::vk::LogicalDevice mLogicalDevice;
    aui::vk::CommandPool mCommandPool;

    struct CharacterData {
        glm::vec4 uv;
    };

    ADeque<CharacterData> mCharData;
    ADeque<FontEntryData> mFontEntryData;

    std::array<glm::vec2, 4> getVerticesForRect(glm::vec2 position, glm::vec2 size);

    void uploadToShaderCommon();
    void identityUv();

    void endDraw(const ABrush& brush);
    void tryEnableFramebuffer(glm::uvec2 windowSize);
    FontEntryData* getFontEntryData(const AFontStyle& fontStyle);
};
