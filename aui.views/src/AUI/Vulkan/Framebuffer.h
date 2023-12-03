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

    struct Framebuffer: public aui::noncopyable {
    public:
        Framebuffer(Instance& instance, VkFramebuffer handle): instance(instance), handle(handle) {} 
        Framebuffer(Instance& instance, VkDevice device, const VkFramebufferCreateInfo& info): instance(instance), device(device), handle([&]{
            VkFramebuffer pool;
            AUI_VK_THROW_ON_ERROR(instance.vkCreateFramebuffer(device, &info, nullptr, &pool));
            return pool;
        }()) {} 

        Framebuffer(Framebuffer&& rhs) noexcept: instance(rhs.instance), device(rhs.device), handle(rhs.handle) {
            rhs.handle = 0;
        }

        ~Framebuffer() {
            if (handle != 0) instance.vkDestroyFramebuffer(device, handle, nullptr);
        }

        operator VkFramebuffer() const noexcept {
            return handle;
        }

    private:
        Instance& instance;
        VkDevice device; 
        VkFramebuffer handle; 
        
    };
}