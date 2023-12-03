// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 12/7/2021.
//

#include <cstddef>
#include <cstdint>
#include "AUI/Vulkan/VulkanRenderer.h"
#include "AUI/Common/AException.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Platform/AProgramModule.h"
#include "AUI/Platform/CommonRenderingContext.h"
#include "AUI/Vulkan/CommandBuffers.h"
#include "AUI/Vulkan/CommandPool.h"
#include "AUI/Vulkan/Fence.h"
#include "AUI/Vulkan/Framebuffer.h"
#include "AUI/Vulkan/Image.h"
#include "AUI/Vulkan/PipelineCache.h"
#include "AUI/Vulkan/RenderPass.h"
#include "AUI/Vulkan/SwapChain.h"
#include "AUI/Vulkan/LogicalDevice.h"
#include "AUI/Vulkan/Instance.h"
#include "AUI/Vulkan/Surface.h"
#include <AUI/GL/gl.h>
#include <AUI/Platform/VulkanRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/GL/GLDebug.h>

static constexpr auto LOG_TAG = "VulkanRenderingContext";


struct VulkanRenderingContext::VulkanObjects {
    AOptional<aui::vk::SurfaceKHR> surface;
    AOptional<aui::vk::SwapChain> swapchain;
    AOptional<aui::vk::CommandBuffers> commandBuffers;
    AVector<aui::vk::Fence> fences;
    AOptional<aui::vk::Image> stencil;
    AOptional<aui::vk::RenderPass> renderPass;
    AOptional<aui::vk::PipelineCache> pipelineCache;
    AVector<aui::vk::Framebuffer> framebuffers;
};

VulkanRenderingContext::VulkanRenderingContext(const ARenderingContextOptions::Vulkan& config) : mConfig(config) {}

VulkanRenderingContext::~VulkanRenderingContext() {
}

void VulkanRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);

    // initialize Xlib window
    static XSetWindowAttributes swa;
    static XVisualInfo* vi;
    if (!vi) {
        XVisualInfo viTemplate;
        aui::zero(viTemplate);

        auto screen = DefaultScreen(ourDisplay);

        viTemplate.visualid = DefaultVisual(ourDisplay, screen)->visualid;
        viTemplate.c_class = TrueColor;

        int count;
        vi = XGetVisualInfo(CommonRenderingContext::ourDisplay,
                            VisualIDMask | VisualClassMask,
                            &viTemplate,
                            &count);
        if (!vi) {
            throw AException("unable to pick visual info");
        }
        auto cmap = XCreateColormap(ourDisplay, ourScreen->root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask
                         | PointerMotionMask | StructureNotifyMask | PropertyChangeMask | StructureNotifyMask;

    }
    initX11Window(init, swa, vi);

    ARender::setRenderer(mRenderer = ourRenderer());

    auto& instance = mRenderer->instance();
    auto& logicalDevice = mRenderer->logicalDevice();
    auto physicalDevice = mRenderer->physicalDevice();

    // create VkSurface
    aui::vk::SurfaceKHR surface(instance, [&] {
        VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
        aui::zero(surfaceCreateInfo);
        surfaceCreateInfo.dpy = CommonRenderingContext::ourDisplay;
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.window = init.window.mHandle; 
        VkSurfaceKHR surface;
        AUI_VK_THROW_ON_ERROR(instance.vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));
        return surface;
    }());


    // pick a surface format
    VkSurfaceFormatKHR selectedFormat; 
    {
        auto supportedFormats = instance.getPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface);
        selectedFormat = supportedFormats.first();

        const VkFormat preferredFormats[] = {
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_R8G8B8A8_UNORM, 
            VK_FORMAT_A8B8G8R8_UNORM_PACK32 
        };

        for (const auto& supportedFormat : supportedFormats) {
            if (ranges::contains(preferredFormats, supportedFormat.format)) {
                selectedFormat = supportedFormat;
                break;
            }
        }
    }
    aui::vk::SwapChain swapchain(instance, physicalDevice, logicalDevice, surface, selectedFormat, {init.width, init.height});
    aui::vk::CommandBuffers commandBuffers(instance, logicalDevice, {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = mRenderer->commandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<std::uint32_t>(swapchain.images().size()),
    });

    auto fences = AVector<aui::vk::Fence>::generate(commandBuffers.size(), [&](size_t i) {
        return aui::vk::Fence(instance, logicalDevice, VkFenceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        });
    });

    const auto stencilFormat = instance.queryStencilOnlyFormat(physicalDevice).valueOrException("unable to find stencilFormat");
    aui::vk::Image stencil(instance, physicalDevice, logicalDevice, {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = stencilFormat,
        .extent = { static_cast<std::uint32_t>(init.width), static_cast<std::uint32_t>(init.height), 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    });

    aui::vk::RenderPass renderPass(instance, logicalDevice, aui::vk::RenderPassCreateInfo{
        .attachments = {
            VkAttachmentDescription{
                .format = selectedFormat.format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            },
            VkAttachmentDescription{
                .format = stencilFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
        },
        .subpasses = {
            aui::vk::SubpassDescription{
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachments = {
                    {
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    }
                },
                .depthStencilAttachment = {
                    {
                        .attachment = 1,
                        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    }
                },
            }
        },
        .dependencies = {
            VkSubpassDependency {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                .dependencyFlags = 0,
            },
            VkSubpassDependency {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dependencyFlags = 0,
            },
        }
    });

    aui::vk::PipelineCache pipelineCache(instance, logicalDevice, VkPipelineCacheCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .initialDataSize = 0,
        .pInitialData = 0,
    });

    auto framebuffers = AVector<aui::vk::Framebuffer>(aui::range(swapchain.images() | ranges::views::transform([&](const auto& image) {
        const VkImageView attachments[] = { image.view, stencil.imageView() };
        return std::move(aui::vk::Framebuffer(instance, logicalDevice, {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .renderPass = renderPass,
            .attachmentCount = std::size(attachments),
            .pAttachments = attachments,
            .width = static_cast<uint32_t>(init.width),
            .height = static_cast<uint32_t>(init.height),
            .layers = 1,
        }));
    })));


    mVulkan->surface = std::move(surface);
    mVulkan->swapchain = std::move(swapchain);
    mVulkan->commandBuffers = std::move(commandBuffers);
    mVulkan->fences = std::move(fences);
    mVulkan->stencil = std::move(stencil);
    mVulkan->renderPass = std::move(renderPass);
    mVulkan->pipelineCache = std::move(pipelineCache);
    mVulkan->framebuffers = std::move(framebuffers);
}

void VulkanRenderingContext::destroyNativeWindow(ABaseWindow& window) {
    CommonRenderingContext::destroyNativeWindow(window);
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        XDestroyWindow(ourDisplay, w->mHandle);
    }
}

void VulkanRenderingContext::beginPaint(ABaseWindow& window) {
    CommonRenderingContext::beginPaint(window);

    if (auto w = dynamic_cast<AWindow*>(&window)) {
    }
    mRenderer->beginPaint(window.getSize());
}

void VulkanRenderingContext::endPaint(ABaseWindow& window) {
    CommonRenderingContext::endPaint(window);
    mRenderer->endPaint();
    if (auto w = dynamic_cast<AWindow*>(&window)) {
    }
}

void VulkanRenderingContext::beginResize(ABaseWindow& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
    }
}

void VulkanRenderingContext::endResize(ABaseWindow& window) {

}

AImage VulkanRenderingContext::makeScreenshot() {
    return AImage();
}

_<VulkanRenderer> VulkanRenderingContext::ourRenderer() {
    static _weak<VulkanRenderer> g;
    if (auto v = g.lock()) {
        return v;
    }
    auto temp = _new<VulkanRenderer>();
    g = temp;
    return temp;
}
