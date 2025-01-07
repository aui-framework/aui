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

    struct Framebuffer: public aui::noncopyable {
    public:
        Framebuffer(const Instance& instance, VkFramebuffer handle): instance(instance), handle(handle) {} 
        Framebuffer(const Instance& instance, VkDevice device, const VkFramebufferCreateInfo& info): instance(instance), device(device), handle([&]{
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
        const Instance& instance;
        VkDevice device; 
        VkFramebuffer handle; 
        
    };
}