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

    struct PipelineCache: public aui::noncopyable {
    public:
        PipelineCache(const Instance& instance, VkPipelineCache handle): instance(instance), handle(handle) {} 
        PipelineCache(const Instance& instance, VkDevice device, const VkPipelineCacheCreateInfo& info): instance(instance), device(device), handle([&]{
            VkPipelineCache pool;
            AUI_VK_THROW_ON_ERROR(instance.vkCreatePipelineCache(device, &info, nullptr, &pool));
            return pool;
        }()) {} 

        PipelineCache(PipelineCache&& rhs) noexcept: instance(rhs.instance), device(rhs.device), handle(rhs.handle) {
            rhs.handle = 0;
        }

        ~PipelineCache() {
            if (handle != 0) instance.vkDestroyPipelineCache(device, handle, nullptr);
        }

        operator VkPipelineCache() const noexcept {
            return handle;
        }

    private:
        const Instance& instance;
        VkDevice device; 
        VkPipelineCache handle; 
        
    };
}