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

#include <optional>

#include <range/v3/view.hpp>
#include <range/v3/algorithm.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "AUI/Common/AException.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Common/AStaticVector.h"
#include "AUI/Traits/values.h"
#include "AUI/Vulkan/Instance.h"


namespace aui::vk {

    struct LogicalDevice: public aui::noncopyable {
    public:
        LogicalDevice(const Instance& instance, VkDevice handle): mInstance(instance), mHandle(handle) {}
        LogicalDevice(const Instance& instance,
                      VkPhysicalDevice physicalDevice,
                      VkPhysicalDeviceFeatures enabledFeatures,
                      VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT,
                      AOptional<VkSurfaceKHR> surface = std::nullopt,
                      void* pNextChain = nullptr)
            : mInstance(instance), mHandle([&] {
                auto queueIndex = instance.queryPhysicalDeviceQueueIndex({
                    .targetDevice = physicalDevice,
                    .supportPresentationOnSurface = surface,
                    .queueFlags = requestedQueueTypes});
                if (!queueIndex) {
                    throw AException("unable to find appropriate queue");
                }


                // Wanted queues needed for logical device creation
                // Because of differing queue family configurations of Vulkan implementation this can be a little bit
                // tricky, especially if the application requests different queue types
                AStaticVector<VkDeviceQueueCreateInfo, 8> queueCreateInfos;

                // Helper function that checks if the queue index is already present in queueCreateInfos, and puts it
                // if not
                auto putIfNotPresent = [&](std::uint32_t index) {
                    if (ranges::find_if(queueCreateInfos, [&](const VkDeviceQueueCreateInfo& rhs) {
                        return rhs.queueFamilyIndex == index;
                    }) != queueCreateInfos.end()) {
                        return;
                    }

                    VkDeviceQueueCreateInfo info;
                    aui::zero(info);
                    info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    info.queueFamilyIndex = index;
                    info.queueCount = 1;
                    const float defaultQueuePriority = 0.f;
                    info.pQueuePriorities = &defaultQueuePriority;
                    queueCreateInfos << info;
                };

                if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
                    mGraphicsQueueIndex = queueIndex;
                    putIfNotPresent(*mGraphicsQueueIndex);
                }
                if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
                    mComputeQueueIndex = queueIndex;

                    putIfNotPresent(*mComputeQueueIndex);
                }
                if (surface) {
                    mPresentationQueueIndex = queueIndex;
                    putIfNotPresent(*mPresentationQueueIndex);
                }

                AVector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
                
                VkDeviceCreateInfo deviceCreateInfo;
                aui::zero(deviceCreateInfo);
                deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
                deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
                deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

                // if a pNext(Chain) has been passed, we need to add it to the device creation info
                VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
                if (pNextChain) {
                    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                    physicalDeviceFeatures2.features = enabledFeatures;
                    physicalDeviceFeatures2.pNext = pNextChain;
                    deviceCreateInfo.pEnabledFeatures = nullptr;
                    deviceCreateInfo.pNext = &physicalDeviceFeatures2;
                }

                deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
                deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();

                VkDevice logicalDevice;
                AUI_VK_THROW_ON_ERROR(instance.vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice));

                return logicalDevice;
              }()) {}

        operator VkDevice() const noexcept {
            return mHandle;
        }

        ~LogicalDevice() {
            mInstance.vkDestroyDevice(mHandle, nullptr);
        }

        [[nodiscard]]
        std::uint32_t graphicsQueueIndex() const noexcept {
            return *mGraphicsQueueIndex;
        }

    private:
        const Instance& mInstance;
        AOptional<std::uint32_t> mGraphicsQueueIndex, mComputeQueueIndex, mPresentationQueueIndex;
        VkDevice mHandle; 
        
    };
}