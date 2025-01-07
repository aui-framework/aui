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

    struct SurfaceKHR: public aui::noncopyable {
    public:
        SurfaceKHR(const Instance& instance, VkSurfaceKHR handle): instance(instance), handle(handle) {} 

        SurfaceKHR(SurfaceKHR&& rhs) noexcept: instance(rhs.instance), handle(rhs.handle) {
            rhs.handle = 0;
        }

        ~SurfaceKHR() {
            if (handle != 0) instance.vkDestroySurfaceKHR(instance, handle, nullptr);
        }

        operator VkSurfaceKHR() const noexcept {
            return handle;
        }

    private:
        const Instance& instance;
        VkSurfaceKHR handle; 
        
    };
}