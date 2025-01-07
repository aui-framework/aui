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

    struct CommandPool: public aui::noncopyable {
    public:
        CommandPool(const Instance& instance, VkCommandPool handle): instance(instance), handle(handle) {} 
        CommandPool(const Instance& instance, VkDevice device, const VkCommandPoolCreateInfo& info): instance(instance), device(device), handle([&]{
            VkCommandPool pool;
            AUI_VK_THROW_ON_ERROR(instance.vkCreateCommandPool(device, &info, nullptr, &pool));
            return pool;
        }()) {} 

        operator VkCommandPool() const noexcept {
            return handle;
        }

        ~CommandPool() {
            instance.vkDestroyCommandPool(device, handle, nullptr);
        }

    private:
        const Instance& instance;
        VkDevice device;
        VkCommandPool handle; 
        
    };
}