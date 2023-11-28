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

#include <optional>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/indices.hpp>
#include "AUI/Vulkan/Instance.h"
#include "AUI/Common/AException.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Logging/ALogger.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>


using namespace aui::vk;

static constexpr auto LOG_TAG = "Vulkan";

template<typename Instance1, typename T>
static AVector<T> vectorGetter(Instance1 instance1, VkResult(*func)(Instance1 instance, uint32_t* count, T* values) ) {
    std::uint32_t count;
    if (auto r = func(instance1, &count, nullptr); r != VK_SUCCESS) {
        throw AException("Vulkan enumerate failed");
    }

    AVector<T> result(count);
    if (auto r = func(instance1, &count, result.data()); r != VK_SUCCESS) {
        throw AException("Vulkan enumerate failed");
    }
    return result;
}

template<typename Instance1, typename Instance2, typename T>
static AVector<T> vectorGetter(Instance1 instance1, Instance2 instance2, VkResult(*func)(Instance1, Instance2, uint32_t* count, T* values) ) {
    std::uint32_t count;
    if (auto r = func(instance1, instance2, &count, nullptr); r != VK_SUCCESS) {
        throw AException("Vulkan enumerate failed");
    }

    AVector<T> result(count);
    if (auto r = func(instance1, instance2, &count, result.data()); r != VK_SUCCESS) {
        throw AException("Vulkan enumerate failed");
    }
    return result;
}

template<typename Instance, typename T>
static AVector<T> vectorGetter(Instance instance, void(*func)(Instance instance, uint32_t* count, T* values) ) {
    std::uint32_t count;
    func(instance, &count, nullptr);

    AVector<T> result(count);
    func(instance, &count, result.data());
    return result;
}



Instance::Instance() {
    std::uint32_t count;
    switch (auto r = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr)) {
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            throw AException("The installed driver is incompatible with Vulkan");
        default:
            throw AException("Unable to get Vulkan extensions: {}"_format(r));

        case VK_SUCCESS:
            break;
    }

    auto availableExtensions = vectorGetter((const char*)nullptr, vkEnumerateInstanceExtensionProperties);
    

    ALogger::info(LOG_TAG) << "Available extensions:";
    for (const auto& extension : availableExtensions) {
        ALogger::info(LOG_TAG) << extension.extensionName;
    }

    VkApplicationInfo appInfo;
    aui::zero(appInfo);
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "AUI";
    appInfo.pEngineName = "aui";
    appInfo.apiVersion = VK_API_VERSION_1_0; 

    std::vector<const char*> extensions = { 
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
        };
    VkInstanceCreateInfo instanceCreateInfo;
    aui::zero(instanceCreateInfo);
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    instanceCreateInfo.enabledExtensionCount = extensions.size();

    if (true)
    {
        const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
        std::uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
        bool validationLayerPresent = false;
        for (VkLayerProperties layer : instanceLayerProperties) {
            if (strcmp(layer.layerName, validationLayerName) == 0) {
                validationLayerPresent = true;
                break;
            }
        }

        if (validationLayerPresent) {
            instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
            instanceCreateInfo.enabledLayerCount = 1;
        } else {
            ALogger::warn(LOG_TAG) << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
        }
    }
    if (auto r = vkCreateInstance(&instanceCreateInfo, nullptr, &instance); r != VK_SUCCESS) {
        throw AException("failed vkCreateInstance");
    }

    *this = instance; // see Instance(VkInstance) ctor for details
}


AVector<VkPhysicalDevice> Instance::enumeratePhysicalDevices() const {
    return vectorGetter(instance, vkEnumeratePhysicalDevices);
}

AVector<VkQueueFamilyProperties> Instance::getPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device) const {
    return vectorGetter(device, vkGetPhysicalDeviceQueueFamilyProperties);
}

AVector<VkSurfaceFormatKHR> Instance::getPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice device, VkSurfaceKHR surface) const {
    return vectorGetter(device, surface, vkGetPhysicalDeviceSurfaceFormatsKHR);
}


AOptional<std::uint32_t> Instance::queryPhysicalDeviceQueueIndex(Instance::QueryPhysicalDeviceQueueIndexConfig config) {
    AOptional<uint32_t> graphicsQueueIndex, presentQueueIndex;
    auto queues = getPhysicalDeviceQueueFamilyProperties(config.targetDevice);

    for (const auto&[i, queue] : queues | ranges::views::enumerate) {
        if ((queue.queueFlags & config.queueFlags) != config.queueFlags) {
            continue;
        }

        if (config.supportPresentationOnSurface) {
            VkBool32 isPresent;
            vkGetPhysicalDeviceSurfaceSupportKHR(config.targetDevice, i, *config.supportPresentationOnSurface, &isPresent);
            
            if (!isPresent) {
                continue;
            }
        }
        
        return i;
    }
    return std::nullopt;
}

AVector<VkImage> Instance::getSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapChain) const {
    return vectorGetter(device, swapChain, vkGetSwapchainImagesKHR);
}