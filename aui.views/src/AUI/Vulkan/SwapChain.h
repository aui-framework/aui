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


#include <range/v3/view.hpp>

#include "AUI/Traits/values.h"
#include "AUI/Vulkan/Instance.h"
#include "AUI/Vulkan/LogicalDevice.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <AUI/Platform/CommonRenderingContext.h>

namespace aui::vk {

    struct SwapChain: public aui::noncopyable {
    public:
        struct Image {
            VkImage image;
            VkImageView view;
        };

        SwapChain(Instance& instance, VkDevice logicalDevice, VkSwapchainKHR handle): mInstance(instance), mLogicalDevice(logicalDevice), mHandle(handle) {}
        SwapChain(Instance& instance,
                  VkPhysicalDevice physicalDevice,
                  VkDevice logicalDevice,
                  VkSurfaceKHR surface,
                  VkSurfaceFormatKHR selectedFormat,
                  glm::uvec2 imageSize,
                  VkSwapchainKHR oldSwapChain = nullptr)
            : mInstance(instance), mLogicalDevice(logicalDevice), mHandle([&] {
                VkSurfaceCapabilitiesKHR surfaceCapabilities;
	            AUI_VK_THROW_ON_ERROR(instance.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

                const uint32_t desiredNumberOfSwapchainImages = surfaceCapabilities.minImageCount + 1;

                VkSwapchainCreateInfoKHR swapchainCI;
                aui::zero(swapchainCI);
                swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                swapchainCI.surface = surface;
                swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
                swapchainCI.imageFormat = selectedFormat.format;
                swapchainCI.imageColorSpace = selectedFormat.colorSpace;
                swapchainCI.imageExtent = {imageSize.x, imageSize.y};
                swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                swapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
                swapchainCI.imageArrayLayers = 1;
                swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                swapchainCI.queueFamilyIndexCount = 0;
                swapchainCI.presentMode = VK_PRESENT_MODE_FIFO_KHR;
                // Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes
                // sure that we can still present already acquired images
                swapchainCI.oldSwapchain = oldSwapChain;
                // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
                swapchainCI.clipped = VK_TRUE;
                swapchainCI.compositeAlpha = [&] {
                  // Find a supported composite alpha format (not all devices
                  // support alpha opaque)
                  VkCompositeAlphaFlagBitsKHR compositeAlpha =
                      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                  // Simply select the first composite alpha format available
                  std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags =
                      {
                          VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                          VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                          VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                          VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
                      };
                  for (auto &compositeAlphaFlag : compositeAlphaFlags) {
                    if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
                      compositeAlpha = compositeAlphaFlag;
                      break;
                    };
                  }
                  return compositeAlpha;
                }();
                VkSwapchainKHR swapChain;
                AUI_VK_THROW_ON_ERROR(instance.vkCreateSwapchainKHR(logicalDevice, &swapchainCI, nullptr, &swapChain));

                return swapChain;
              }()) {
                auto images = instance.getSwapchainImagesKHR(logicalDevice, mHandle); 
                mImages = images | ranges::views::transform([&](VkImage image) {
                    VkImageViewCreateInfo colorAttachmentView = {};
                    colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    colorAttachmentView.pNext = NULL;
                    colorAttachmentView.format = selectedFormat.format;
                    colorAttachmentView.components = {
                        VK_COMPONENT_SWIZZLE_R,
                        VK_COMPONENT_SWIZZLE_G,
                        VK_COMPONENT_SWIZZLE_B,
                        VK_COMPONENT_SWIZZLE_A
                    };
                    colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    colorAttachmentView.subresourceRange.baseMipLevel = 0;
                    colorAttachmentView.subresourceRange.levelCount = 1;
                    colorAttachmentView.subresourceRange.baseArrayLayer = 0;
                    colorAttachmentView.subresourceRange.layerCount = 1;
                    colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    colorAttachmentView.flags = 0;

                    colorAttachmentView.image = image;
                    VkImageView view;
                    AUI_VK_THROW_ON_ERROR(instance.vkCreateImageView(logicalDevice, &colorAttachmentView, nullptr, &view));
                    return Image {
                        .image = image,
                        .view = view,
                    };
                }) | ranges::to_vector;
              }

        operator VkSwapchainKHR() const noexcept {
            return mHandle;
        }

        ~SwapChain() {
            for (const auto& image : mImages) {
                mInstance.vkDestroyImageView(mLogicalDevice, image.view, nullptr);
            }
            mInstance.vkDestroySwapchainKHR(mLogicalDevice, mHandle, nullptr);
        }        

        [[nodiscard]]
        const std::vector<Image>& images() const noexcept {
            return mImages;
        }

    private:
        Instance& mInstance;
        VkDevice mLogicalDevice;
        VkSwapchainKHR mHandle; 
        std::vector<Image> mImages;
    };
}