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

//
// Created by Alex2772 on 11/19/2021.
//

#include "VulkanRenderer.h"
#include "AUI/Common/AException.h"
#include "AUI/GL/Framebuffer.h"
#include "AUI/GL/GLEnums.h"
#include "AUI/GL/Program.h"
#include "AUI/GL/Texture2D.h"
#include "AUI/Render/Brush/Gradient.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Render/ITexture.h"
#include "AUI/Util/AAngleRadians.h"
#include "AUI/Vulkan/Instance.h"
#include "glm/fwd.hpp"
#include <AUI/Traits/callables.h>
#include <AUI/Platform/AFontManager.h>
#include <AUI/GL/Vbo.h>
#include <AUI/GL/State.h>
#include <AUI/GL/RenderTarget/RenderbufferRenderTarget.h>
#include <AUI/Platform/AWindowBase.h>
#include <AUI/Logging/ALogger.h>
#include <AUISL/Generated/basic.vsh.glsl120.h>
#include <AUISL/Generated/basic_uv.vsh.glsl120.h>
#include <AUISL/Generated/shadow.fsh.glsl120.h>
#include <AUISL/Generated/shadow_inner.fsh.glsl120.h>
#include <AUISL/Generated/rect_solid.fsh.glsl120.h>
#include <AUISL/Generated/rect_solid_rounded.fsh.glsl120.h>
#include <AUISL/Generated/rect_gradient.fsh.glsl120.h>
#include <AUISL/Generated/rect_gradient_rounded.fsh.glsl120.h>
#include <AUISL/Generated/rect_textured.fsh.glsl120.h>
#include <AUISL/Generated/border_rounded.fsh.glsl120.h>
#include <AUISL/Generated/symbol.vsh.glsl120.h>
#include <AUISL/Generated/symbol.fsh.glsl120.h>
#include <AUISL/Generated/symbol_sub.fsh.glsl120.h>
#include <AUISL/Generated/square_sector.fsh.glsl120.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <vulkan/vulkan_core.h>

static constexpr auto LOG_TAG = "VulkanRenderer";

template <typename C>
concept AuiSLShader = requires(C&& c) {
    { C::code() } -> std::same_as<const char*>;
    C::setup(0);
};

template <AuiSLShader Vertex, AuiSLShader Fragment>
inline void useAuislShader(gl::Program& out) {
    out.loadRaw(Vertex::code(), Fragment::code());
    Vertex::setup(out.handle());
    Fragment::setup(out.handle());
    out.compile();
}

namespace {
class VulkanTexture2D : public ITexture {
public:
    void setImage(AImageView image) override {}
};
}   // namespace

VulkanRenderer::VulkanRenderer()
  : mPhysicalDevice([this] {
      // find applicable device (first by default)
      auto devices = mInstance.enumeratePhysicalDevices();
      if (devices.empty()) {
          throw AException("no compatible Vulkan device found");
      }

      auto& physicalDevice = devices.first();
      {
          VkPhysicalDeviceProperties deviceProperties;
          (*mInstance.vkGetPhysicalDeviceProperties)(physicalDevice, &deviceProperties);
          ALogger::info(LOG_TAG) << "Using device: " << deviceProperties.deviceName;
      }
      return physicalDevice;
  }())
  , mLogicalDevice(mInstance, mPhysicalDevice, {}, VK_QUEUE_GRAPHICS_BIT)
  , mCommandPool(
        mInstance, mLogicalDevice,
        VkCommandPoolCreateInfo {
          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
          .pNext = nullptr,
          .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          .queueFamilyIndex = mLogicalDevice.graphicsQueueIndex(),
        }) {}

glm::mat4 VulkanRenderer::getProjectionMatrix() const {
    return glm::ortho(
        0.0f, static_cast<float>(mWindow->getWidth()) - 0.0f, static_cast<float>(mWindow->getHeight()) - 0.0f, 0.0f,
        -1.f, 1.f);
}

void VulkanRenderer::uploadToShaderCommon() {}

std::array<glm::vec2, 4> VulkanRenderer::getVerticesForRect(glm::vec2 position, glm::vec2 size) {
    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    auto apply = [&](glm::vec4 v) {
        auto result = mTransform * v;
        return glm::vec3(result) / result.w;
    };

    return {
        glm::vec2 {
          x,
          h,
        },
        glm::vec2 {
          w,
          h,
        },
        glm::vec2 {
          x,
          y,
        },
        glm::vec2 {
          w,
          y,
        },
    };
}
void VulkanRenderer::rectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size) {}

void VulkanRenderer::rectImpl(glm::vec2 position, glm::vec2 size) {}

void VulkanRenderer::identityUv() {}

void VulkanRenderer::roundedRectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius) {}

void VulkanRenderer::rectangleBorder(const ABrush& brush, glm::vec2 position, glm::vec2 size, float lineWidth) {}

void VulkanRenderer::roundedRectangleBorder(
    const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius, int borderWidth) {}

void VulkanRenderer::boxShadow(glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) {
    assert(("blurRadius is expected to be non negative, use drawBoxShadowInner for inset shadows instead",
            blurRadius >= 0.f));
}

void VulkanRenderer::boxShadowInner(
    glm::vec2 position, glm::vec2 size, float blurRadius, float spreadRadius, float borderRadius, const AColor& color,
    glm::vec2 offset) {
    assert(("blurRadius is expected to be non negative", blurRadius >= 0.f));
    blurRadius *= -1.f;
}
void VulkanRenderer::string(glm::vec2 position, const AString& string, const AFontStyle& fs) {
    prerenderString(position, string, fs)->draw();
}

void VulkanRenderer::endDraw(const ABrush& brush) {}

void VulkanRenderer::setBlending(Blending blending) {}
_<IRenderer::IPrerenderedString>
VulkanRenderer::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    return nullptr;
}

VulkanRenderer::FontEntryData* VulkanRenderer::getFontEntryData(const AFontStyle& fontStyle) { return nullptr; }

std::unique_ptr<ITexture> VulkanRenderer::createNewTexture() { return std::make_unique<VulkanTexture2D>(); }

_<IRenderer::IMultiStringCanvas> VulkanRenderer::newMultiStringCanvas(const AFontStyle& style) {
    class VulkanMultiStringCanvas : public IRenderer::IMultiStringCanvas {
    public:
        void addString(const glm::ivec2& position, const AString& text) noexcept override {}

        _<IRenderer::IPrerenderedString> finalize() noexcept override { return nullptr; }
    };
    return _new<VulkanMultiStringCanvas>();
}

void VulkanRenderer::pushMaskBefore() {}

void VulkanRenderer::pushMaskAfter() {}

void VulkanRenderer::popMaskBefore() {}

void VulkanRenderer::popMaskAfter() {}
void VulkanRenderer::bindViewport() const noexcept {}

void VulkanRenderer::squareSector(
    const ABrush& brush, const glm::vec2& position, const glm::vec2& size, AAngleRadians begin, AAngleRadians end) {}

void VulkanRenderer::tryEnableFramebuffer(glm::uvec2 windowSize) {}

void VulkanRenderer::beginPaint(glm::uvec2 windowSize) {}

void VulkanRenderer::endPaint() {}

uint32_t VulkanRenderer::getSupersamplingRatio() const noexcept { return 1; }

void VulkanRenderer::lines(
    const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {}
void VulkanRenderer::points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) {}
void VulkanRenderer::lines(
    const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) {

}
_unique<IRenderViewToTexture> VulkanRenderer::newRenderViewToTexture() noexcept {
    return _unique<IRenderViewToTexture>();
}
