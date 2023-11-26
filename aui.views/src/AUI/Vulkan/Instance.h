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

#pragma once

#include "AUI/Platform/AProgramModule.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <AUI/Platform/CommonRenderingContext.h>

#if AUI_PLATFORM_LINUX
#include <vulkan/vulkan_xlib.h>
#endif

namespace vk {
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


        operator VkInstance() const noexcept {
            return instance;
        }

        _<AProgramModule> lib = AProgramModule::load(VULKAN_DEFAULT_PATH, { .decorateName = false });

        VkInstance instance = nullptr;

        VkAllocationCallbacks allorator;

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

#undef DEF_PROC
    };
}