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
#include "AUI/Traits/values.h"
#include "AUI/Vulkan/DeviceMemory.h"
#include "AUI/Vulkan/Instance.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <AUI/Platform/CommonRenderingContext.h>

namespace aui::vk {

    struct Image: public aui::noncopyable {
    public:
        Image(const Instance& instance, VkImage handle): instance(instance), handle(handle) {} 
        Image(const Instance& instance, VkPhysicalDevice physicalDevice, VkDevice device, const VkImageCreateInfo& info): instance(instance), device(device), handle([&]{
            VkImage pool;
            AUI_VK_THROW_ON_ERROR(instance.vkCreateImage(device, &info, nullptr, &pool));
            return pool;
        }()) {
            VkMemoryRequirements reqs;
            instance.vkGetImageMemoryRequirements(device, handle, &reqs);
            
            memory.emplace(instance, device, VkMemoryAllocateInfo {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .pNext = nullptr,
                .allocationSize = reqs.size,
                .memoryTypeIndex = instance.getMemoryType(physicalDevice, reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).valueOrException("unable to pick memory format for an image storage"),
            });

            AUI_VK_THROW_ON_ERROR(instance.vkBindImageMemory(device, handle, *memory, 0));


            VkImageViewCreateInfo imageViewCI{};
            imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCI.image = handle;
            imageViewCI.format = info.format;
            imageViewCI.subresourceRange.baseMipLevel = 0;
            imageViewCI.subresourceRange.levelCount = 1;
            imageViewCI.subresourceRange.baseArrayLayer = 0;
            imageViewCI.subresourceRange.layerCount = 1;

            if (info.format == VK_FORMAT_S8_UINT) {
                imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            } else {
                imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
                if (info.format >= VK_FORMAT_D16_UNORM_S8_UINT) {
                    imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            }
            VkImageView imageView;
            AUI_VK_THROW_ON_ERROR(instance.vkCreateImageView(device, &imageViewCI, nullptr, &imageView));
            mImageView = imageView;
        } 

        Image(Image&& rhs) noexcept: instance(rhs.instance), device(rhs.device), handle(rhs.handle), memory(std::move(rhs.memory)), mImageView(std::move(rhs.mImageView)) {
            rhs.handle = 0;
        }

        ~Image() {
            if (handle == 0) {
                return;
            }
            memory.reset();
            if (mImageView) {
                instance.vkDestroyImageView(device, *mImageView, nullptr);
            }
            instance.vkDestroyImage(device, handle, nullptr);
        }

        [[nodiscard]]
        VkImageView imageView() const noexcept {
            return *mImageView;
        }

        operator VkImage() const noexcept {
            return handle;
        }

    private:
        const Instance& instance;
        VkDevice device; 
        VkImage handle; 
        AOptional<DeviceMemory> memory;
        AOptional<VkImageView> mImageView;
        
    };
}