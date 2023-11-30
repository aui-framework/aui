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
        Image(Instance& instance, VkImage handle): instance(instance), handle(handle) {} 
        Image(Instance& instance, VkDevice device, const VkImageCreateInfo& info): instance(instance), device(device), handle([&]{
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
                .memoryTypeIndex = instance.getMemoryType(reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).valueOrException("unable to pick memory format for an image storage"),
            });

            AUI_VK_THROW_ON_ERROR(instance.vkBindImageMemory(device, handle, *memory, 0));
        } 

        Image(Image&& rhs) noexcept: instance(rhs.instance), device(rhs.device), handle(rhs.handle) {
            rhs.handle = 0;
        }

        ~Image() {
            memory.reset();
            if (handle != 0) instance.vkDestroyImage(device, handle, nullptr);
        }

        operator VkImage() const noexcept {
            return handle;
        }

    private:
        Instance& instance;
        VkDevice device; 
        VkImage handle; 
        AOptional<DeviceMemory> memory;
        
    };
}