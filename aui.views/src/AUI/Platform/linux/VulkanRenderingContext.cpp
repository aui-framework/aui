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
// Created by Alex2772 on 12/7/2021.
//

#include <cstddef>
#include <cstdint>
#include <limits>
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
#include "AUI/Vulkan/Semaphore.h"
#include "AUI/Vulkan/LogicalDevice.h"
#include "AUI/Vulkan/Instance.h"
#include "AUI/Vulkan/Surface.h"
#include <AUI/GL/gl.h>
#include <AUI/Platform/VulkanRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/GL/GLDebug.h>
#include <vulkan/vulkan_core.h>

static constexpr auto LOG_TAG = "VulkanRenderingContext";


struct VulkanRenderingContext::VulkanObjects {
    aui::vk::SurfaceKHR surface;
    aui::vk::SwapChain swapchain;
    aui::vk::CommandBuffers commandBuffers;
    AVector<aui::vk::Fence> fences;
    aui::vk::Image stencil;
    aui::vk::RenderPass renderPass;
    aui::vk::PipelineCache pipelineCache;
    AVector<aui::vk::Framebuffer> framebuffers;
    aui::vk::Semaphore presentCompleteSemaphore;
    aui::vk::Semaphore renderCompleteSemaphore;
    VkQueue graphicsQueue;
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

    mRenderer = ourRenderer();

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

    aui::vk::Semaphore presentCompleteSemaphore(instance, logicalDevice);
    aui::vk::Semaphore renderCompleteSemaphore(instance, logicalDevice);

    VkQueue graphicsQueue;
    instance.vkGetDeviceQueue(logicalDevice, logicalDevice.graphicsQueueIndex(), 0, &graphicsQueue);

    *mVulkan = VulkanObjects {
        .surface = std::move(surface),
        .swapchain = std::move(swapchain),
        .commandBuffers = std::move(commandBuffers),
        .fences = std::move(fences),
        .stencil = std::move(stencil),
        .renderPass = std::move(renderPass),
        .pipelineCache = std::move(pipelineCache),
        .framebuffers = std::move(framebuffers),
        .presentCompleteSemaphore = std::move(presentCompleteSemaphore),
        .renderCompleteSemaphore = std::move(renderCompleteSemaphore),
        .graphicsQueue = graphicsQueue,
    };

}

void VulkanRenderingContext::destroyNativeWindow(AWindowBase& window) {
    CommonRenderingContext::destroyNativeWindow(window);
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        XDestroyWindow(ourDisplay, w->mHandle);
    }
}

void VulkanRenderingContext::beginPaint(AWindowBase& window) {
    CommonRenderingContext::beginPaint(window);

    const VkDevice device = mRenderer->logicalDevice();
    const auto& instance = mRenderer->instance();

    // Use a fence to wait until the command buffer has finished execution before using it again.
    VkFence currentFence = vulkan().fences[mCurrentFrame];
    AUI_VK_THROW_ON_ERROR(instance.vkWaitForFences(device, 1, &currentFence, VK_TRUE, UINT64_MAX));

    // Acquires the next image in the swap chain.
    // The function will always wait until the next image has been acquired by setting timeout to UINT64_MAX.
    // Note that the implementation is free to return the images in any order, so we must use the acquire function and
    // can't just cycle through the images.
    auto result = instance.vkAcquireNextImageKHR(device,
                                                           vulkan().swapchain,
                                                           std::numeric_limits<std::uint64_t>::max(),
                                                           vulkan().presentCompleteSemaphore,
                                                           nullptr,
                                                           &mImageIndex);

    switch (result) {
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
            // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE)
            // SRS - If no longer optimal (VK_SUBOPTIMAL_KHR), wait until submitFrame() in case number of swapchain
            // images will change on resize
            recreateObjectsDueToResize();
            break;
        default:
            AUI_VK_THROW_ON_ERROR(result);
    }
    
    AUI_VK_THROW_ON_ERROR(instance.vkResetFences(device, 1, &currentFence));


    // Build the command buffer
    // Unlike in OpenGL all rendering commands are recorded into command buffers that are then submitted to the queue
    // This allows to generate work upfront in a separate thread
    // However AUI is mostly singlethread, so we not using such feature
    const VkCommandBuffer currentCommandBuffer = vulkan().commandBuffers[0];
    AUI_VK_THROW_ON_ERROR(instance.vkResetCommandBuffer(currentCommandBuffer, 0));


    {
		VkCommandBufferBeginInfo cmdBufInfo{};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		AUI_VK_THROW_ON_ERROR(instance.vkBeginCommandBuffer(currentCommandBuffer, &cmdBufInfo));
    }

    {
		// Set clear values for all framebuffer attachments with loadOp set to clear
		// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to
        // set clear values for both
		VkClearValue clearValues[2];
        aui::zero(clearValues);
		clearValues[0].color = { { 1.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
        aui::zero(renderPassBeginInfo);
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = vulkan().renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = window.getWidth();
		renderPassBeginInfo.renderArea.extent.height = window.getHeight();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = vulkan().framebuffers[mImageIndex];

        // Start the first sub pass specified in our default render pass setup by the base class
		// This will clear the color and depth attachment
		instance.vkCmdBeginRenderPass(currentCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    if (auto w = dynamic_cast<AWindow*>(&window)) {
    }
    mRenderer->beginPaint(window.getSize());
}

void VulkanRenderingContext::endPaint(AWindowBase& window) {
    CommonRenderingContext::endPaint(window);
    mRenderer->endPaint();

    const auto& instance = mRenderer->instance();
    const VkCommandBuffer currentCommandBuffer = vulkan().commandBuffers[0];
    instance.vkCmdEndRenderPass(currentCommandBuffer);

    // Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system
    AUI_VK_THROW_ON_ERROR(instance.vkEndCommandBuffer(currentCommandBuffer));

    const VkSemaphore renderCompleteSemaphore = vulkan().renderCompleteSemaphore;
    const VkSemaphore presentCompleteSemaphore = vulkan().presentCompleteSemaphore;

	// Submit the command buffer to the graphics queue
    {
		// Pipeline stage at which the queue submission will wait (via renderCompleteSemaphores)
		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		// The submit info structure specifies a command buffer queue submission batch
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitStageMask;      // Pointer to the list of pipeline stages that the semaphore waits will occur at
		submitInfo.waitSemaphoreCount = 1;                  // One wait semaphore
		submitInfo.signalSemaphoreCount = 1;                // One signal semaphore
		submitInfo.pCommandBuffers = &currentCommandBuffer; // Command buffers(s) to execute in this batch (submission)
		submitInfo.commandBufferCount = 1;                  // One command buffer

		// Semaphore to wait upon before the submitted command buffer starts executing
		submitInfo.pWaitSemaphores = &presentCompleteSemaphore; 
		// Semaphore to be signaled when command buffers have completed
		submitInfo.pSignalSemaphores = &renderCompleteSemaphore;

		// Submit to the graphics queue passing a wait fence
		AUI_VK_THROW_ON_ERROR(instance.vkQueueSubmit(vulkan().graphicsQueue, 1, &submitInfo, vulkan().fences[mCurrentFrame]));
    }

    {
        const VkSwapchainKHR swapchain = vulkan().swapchain;
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = NULL;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &mImageIndex;

        // Check if a wait semaphore has been specified to wait for before presenting the image
        if (renderCompleteSemaphore != VK_NULL_HANDLE)
        {
            presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
            presentInfo.waitSemaphoreCount = 1;
        }

        // Present the current frame buffer to the swap chain
        // Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for
        // swap chain presentation
        // This ensures that the image is not presented to the windowing system until all commands have been submitted
        auto result = mRenderer->instance().vkQueuePresentKHR(vulkan().graphicsQueue, &presentInfo);
        switch (result) {
            case VK_ERROR_OUT_OF_DATE_KHR:
            case VK_SUBOPTIMAL_KHR:
                // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE)
                // SRS - If no longer optimal (VK_SUBOPTIMAL_KHR), wait until submitFrame() in case number of swapchain
                // images will change on resize
                recreateObjectsDueToResize();
                break;
            default:
                AUI_VK_THROW_ON_ERROR(result);
        }
    }

    if (auto w = dynamic_cast<AWindow*>(&window)) {
    }
}

void VulkanRenderingContext::beginResize(AWindowBase& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
    }
}

void VulkanRenderingContext::endResize(AWindowBase& window) {

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
void VulkanRenderingContext::recreateObjectsDueToResize() {
}

const VulkanRenderingContext::VulkanObjects& VulkanRenderingContext::vulkan() const noexcept { return **mVulkan; }
IRenderer& VulkanRenderingContext::renderer() { return *mRenderer; }
