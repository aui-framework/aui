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

    struct Fence: public aui::noncopyable {
    public:
        Fence(const Instance& instance, VkFence handle): instance(instance), handle(handle) {} 
        Fence(const Instance& instance, VkDevice device, const VkFenceCreateInfo& info): instance(instance), device(device), handle([&]{
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
        const Instance& instance;
        VkDevice device; 
        VkFence handle; 
        
    };
}