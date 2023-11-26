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

#include <range/v3/algorithm/contains.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/iterator/operations.hpp>
#include <range/v3/view.hpp>
#include "AUI/Common/AException.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Platform/AProgramModule.h"
#include "AUI/Platform/CommonRenderingContext.h"
#include "AUI/Vulkan/Instance.h"
#include <AUI/GL/gl.h>
#include <AUI/Platform/VulkanRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/GL/GLDebug.h>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/indices.hpp>
#include <vulkan/vulkan_core.h>

static constexpr auto LOG_TAG = "VulkanRenderingContext";

VulkanRenderingContext::~VulkanRenderingContext() {
}

void VulkanRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);

    // initialize VkInstance
    vk::Instance instance;

    // find applicable device (first by default)
    auto devices = instance.enumeratePhysicalDevices();
    if (devices.empty()) {
        throw AException("no compatible Vulkan device found");
    }

    auto& targetDevice = devices.first();
    {
        VkPhysicalDeviceProperties deviceProperties;
        (*instance.vkGetPhysicalDeviceProperties)(targetDevice, &deviceProperties);
        ALogger::info(LOG_TAG) << "Using device: " << deviceProperties.deviceName;
    }


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

    // create VkSurface
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
    aui::zero(surfaceCreateInfo);
    surfaceCreateInfo.dpy = CommonRenderingContext::ourDisplay;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.window = init.window.mHandle; 
    VkSurfaceKHR surface;
    if (auto r = instance.vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface); r != VK_SUCCESS) {
        throw AException("unable to create Vulkan surface");
    }

    // find presentation and graphics queues

    AOptional<uint32_t> graphicsQueueIndex, presentQueueIndex;
    {
        auto queues = instance.getPhysicalDeviceQueueFamilyProperties(targetDevice);

        for (const auto&[i, queue] : queues | ranges::views::enumerate) {
            bool isGraphics = (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
            VkBool32 isPresent;
            instance.vkGetPhysicalDeviceSurfaceSupportKHR(targetDevice, i, surface, &isPresent);

            if (isGraphics && isPresent) {
                // found a queue that supports both - using it even if some index is already set
                graphicsQueueIndex = i;
                presentQueueIndex = i;
                break;
            }

            if (isGraphics && !graphicsQueueIndex) {
                graphicsQueueIndex = i;
            }

            if (isPresent  && !presentQueueIndex) {
                presentQueueIndex = i;
            }
        }

        if (!graphicsQueueIndex || !presentQueueIndex) {
            throw AException("unable to find graphics and/or present queue");
        }

        // temporary - we do not support different graphics and present queues
        if (*graphicsQueueIndex != *presentQueueIndex) {
            throw AException("graphics and present queue are different - we do not support that");
        }
    }

    // pick a surface format
    VkSurfaceFormatKHR selectedFormat; 
    {
        auto supportedFormats = instance.getPhysicalDeviceSurfaceFormatsKHR(targetDevice, surface);
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


    ARender::setRenderer(mRenderer = ourRenderer());

#if defined(_DEBUG)
    gl::setupDebug();
#endif
    //assert(glGetError() == 0);

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
