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

#include "AUI/Traits/values.h"
#include "AUI/Vulkan/Instance.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <AUI/Platform/CommonRenderingContext.h>

namespace aui::vk {

    struct Fence: public aui::noncopyable {
    public:
        Fence(Instance& instance, VkFence handle): instance(instance), handle(handle) {} 
        Fence(Instance& instance, VkDevice device, const VkFenceCreateInfo& info): instance(instance), device(device), handle([&]{
            VkFence pool;
            AUI_VK_THROW_ON_ERROR(instance.vkCreateFence(device, &info, nullptr, &pool));
            return pool;
        }()) {} 

        Fence(Fence&& rhs) noexcept: instance(rhs.instance), device(rhs.device), handle(rhs.handle) {
            rhs.handle = 0;
        }

        ~Fence() {
            if (handle != 0) instance.vkDestroyFence(device, handle, nullptr);
        }

        operator VkFence() const noexcept {
            return handle;
        }

    private:
        Instance& instance;
        VkDevice device; 
        VkFence handle; 
        
    };
}