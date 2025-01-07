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