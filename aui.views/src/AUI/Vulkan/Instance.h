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

#include "AUI/Common/AOptional.h"
#include "AUI/Platform/AProgramModule.h"
#include "AUI/Traits/values.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <AUI/Platform/CommonRenderingContext.h>

#if AUI_PLATFORM_LINUX
#include <vulkan/vulkan_xlib.h>
#endif

#define AUI_VK_THROW_ON_ERROR(expr) if (auto r = expr; r != VK_SUCCESS) throw AException(#expr " failed: {}"_format((int)r))
namespace aui::vk {
#ifdef __OpenBSD__
    static constexpr auto VULKAN_DEFAULT_PATH = "libvulkan.so";
#else
    static constexpr auto VULKAN_DEFAULT_PATH = "libvulkan.so.1";
#endif

    struct Instance {
    public:
        Instance();
        Instance(VkInstance instance): instance(instance) {} // updates function pointers, as instance is present

        AVector<VkPhysicalDevice> enumeratePhysicalDevices() const;
        AVector<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device) const;
        AVector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice device, VkSurfaceKHR surface) const;
        AVector<VkImage> getSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapChain) const;

        /**
         * @brief Picks the supported depth stencil format.
         * @param physicalDevice physical device
         * @return applicable format, or nullopt if not found
         */
        AOptional<VkFormat> queryStencilOnlyFormat(VkPhysicalDevice physicalDevice) const noexcept;
        
        /**
         * @brief Picks the supported depth stencil format from passed formats.
         * @param physicalDevice physical device
         * @param formats formats to pick from
         * @return applicable format, or nullopt if not found
         * @details
         * Use queryStencilOnlyFormat or similar function to pick from the predefined formats.
         */
        AOptional<VkFormat> querySupportedFormat(VkPhysicalDevice physicalDevice, std::span<VkFormat> formats) const noexcept;

        /**
        * @brief Get the index of a memory type that has all the requested property bits set.
        *
        * @param physicalDevice physical device
        * @param typeBits Bit mask with bits set for each memory type supported by the resource to request for (from VkMemoryRequirements)
        * @param properties Bit mask of properties for the memory type to request
        * 
        * @return Index of the requested memory type, or nullopt if not found
        */
        AOptional<uint32_t> getMemoryType(VkPhysicalDevice physicalDevice, VkSparseMemoryBindFlags typeBits, VkMemoryPropertyFlags properties) const noexcept;
       

        operator VkInstance() const noexcept {
            return instance;
        }

        _<AProgramModule> lib = AProgramModule::load(VULKAN_DEFAULT_PATH, { .decorateName = false });

        VkInstance instance = nullptr;

        struct QueryPhysicalDeviceQueueIndexConfig {
            VkPhysicalDevice targetDevice;
            AOptional<VkSurfaceKHR> supportPresentationOnSurface;
            VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
        };
        AOptional<std::uint32_t> queryPhysicalDeviceQueueIndex(aui::vk::Instance::QueryPhysicalDeviceQueueIndexConfig config) const;

#define DEF_PROC(name) PFN_ ## name name = (PFN_ ## name ) lib->getProcAddressRawPtr(#name);
#define DEF_PROC_VK(name) PFN_ ## name name = (PFN_ ## name ) vkGetInstanceProcAddr(instance, #name);

        DEF_PROC(vkGetInstanceProcAddr)
        DEF_PROC_VK(vkEnumerateInstanceExtensionProperties)
        DEF_PROC_VK(vkCreateXlibSurfaceKHR)
        DEF_PROC_VK(vkEnumeratePhysicalDevices)
        DEF_PROC_VK(vkCreateInstance)
        DEF_PROC_VK(vkEnumerateInstanceLayerProperties)
        DEF_PROC_VK(vkGetPhysicalDeviceProperties)
        DEF_PROC_VK(vkGetPhysicalDeviceQueueFamilyProperties)
        DEF_PROC_VK(vkGetPhysicalDeviceSurfaceSupportKHR)
        DEF_PROC_VK(vkGetPhysicalDeviceSurfaceFormatsKHR)
        DEF_PROC_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
        DEF_PROC_VK(vkGetPhysicalDeviceMemoryProperties)
        DEF_PROC_VK(vkGetPhysicalDeviceFormatProperties)
        DEF_PROC_VK(vkDestroySurfaceKHR)
        DEF_PROC_VK(vkAcquireNextImageKHR)
        DEF_PROC_VK(vkGetDeviceQueue)
        DEF_PROC_VK(vkQueuePresentKHR)
        DEF_PROC_VK(vkQueueSubmit)

        DEF_PROC_VK(vkCreateSemaphore)
        DEF_PROC_VK(vkDestroySemaphore)

        DEF_PROC_VK(vkCreateCommandPool)
        DEF_PROC_VK(vkDestroyCommandPool)

        DEF_PROC_VK(vkCreateDevice)
        DEF_PROC_VK(vkDestroyDevice)

        DEF_PROC_VK(vkCreateSwapchainKHR)
        DEF_PROC_VK(vkDestroySwapchainKHR)
        DEF_PROC_VK(vkGetSwapchainImagesKHR)

        DEF_PROC_VK(vkCreateImageView)
        DEF_PROC_VK(vkDestroyImageView)

        DEF_PROC_VK(vkAllocateCommandBuffers)
        DEF_PROC_VK(vkFreeCommandBuffers)
        DEF_PROC_VK(vkResetCommandBuffer)
        DEF_PROC_VK(vkBeginCommandBuffer)
        DEF_PROC_VK(vkEndCommandBuffer)

        DEF_PROC_VK(vkCreateFence)
        DEF_PROC_VK(vkDestroyFence)
        DEF_PROC_VK(vkWaitForFences)
        DEF_PROC_VK(vkResetFences)

        DEF_PROC_VK(vkCreateImage)
        DEF_PROC_VK(vkGetImageMemoryRequirements)
        DEF_PROC_VK(vkDestroyImage)

        DEF_PROC_VK(vkAllocateMemory)
        DEF_PROC_VK(vkBindImageMemory)
        DEF_PROC_VK(vkFreeMemory)

        DEF_PROC_VK(vkCreateRenderPass)
        DEF_PROC_VK(vkDestroyRenderPass)
        DEF_PROC_VK(vkCmdBeginRenderPass)
        DEF_PROC_VK(vkCmdEndRenderPass)

        DEF_PROC_VK(vkCreatePipelineCache)
        DEF_PROC_VK(vkDestroyPipelineCache)

        DEF_PROC_VK(vkCreateFramebuffer)
        DEF_PROC_VK(vkDestroyFramebuffer)

#undef DEF_PROC
    };
}