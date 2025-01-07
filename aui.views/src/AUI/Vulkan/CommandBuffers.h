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

#include <AUI/Platform/CommonRenderingContext.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "AUI/Traits/values.h"
#include "AUI/Vulkan/Instance.h"

namespace aui::vk {

struct CommandBuffers : public aui::noncopyable, public AVector<VkCommandBuffer> {
   public:
    CommandBuffers(const Instance& instance, VkDevice device, VkCommandBufferAllocateInfo info)
        : mInstance(instance), mDevice(device), mCommandPool(info.commandPool) {
        resize(info.commandBufferCount);
        AUI_VK_THROW_ON_ERROR(instance.vkAllocateCommandBuffers(device, &info, data()));
    }
    CommandBuffers(CommandBuffers&& rhs) noexcept
        : AVector<VkCommandBuffer>(std::move(rhs)),
          mInstance(rhs.mInstance),
          mDevice(rhs.mDevice),
          mCommandPool(rhs.mCommandPool) {
        rhs.mCommandPool = 0;
    }

    ~CommandBuffers() {
        if (mCommandPool == 0) {
            return;
        }
        mInstance.vkFreeCommandBuffers(mDevice, mCommandPool, size(), data());
    }

   private:
    const Instance& mInstance;
    VkDevice mDevice;
    VkCommandPool mCommandPool;
};
}   // namespace aui::vk