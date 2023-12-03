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
#include "AUI/Common/ASmallVector.h"
#include "AUI/Traits/values.h"
#include "AUI/Vulkan/Instance.h"
#include <cstdio>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <AUI/Platform/CommonRenderingContext.h>

namespace aui::vk {

    /**
     * @brief Helper initializer struct that transforms to VkRenderPassCreateInfo.
     */
    struct RenderPassCreateInfo {
        ASmallVector<VkAttachmentDescription, 8> attachments;
        ASmallVector<VkSubpassDescription, 8> subpasses;
        ASmallVector<VkSubpassDependency, 8> dependencies;


        operator VkRenderPassCreateInfo() const noexcept {
            return VkRenderPassCreateInfo {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = static_cast<uint32_t>(subpasses.size()),
                .pSubpasses = subpasses.data(),
                .dependencyCount = static_cast<uint32_t>(dependencies.size()),
                .pDependencies = dependencies.data(),
            };
        };
    };

    /**
     * @brief Helper initializer struct that transforms to VkSubpassDescription.
     */
    struct SubpassDescription {
        VkSubpassDescriptionFlags              flags;
        VkPipelineBindPoint                    pipelineBindPoint;
        ASmallVector<VkAttachmentReference, 8> inputAttachments;
        ASmallVector<VkAttachmentReference, 8> colorAttachments;
        ASmallVector<VkAttachmentReference, 8> resolveAttachments;
        AOptional<VkAttachmentReference>       depthStencilAttachment;
        ASmallVector<std::uint32_t, 8>         preserveAttachments;

        operator VkSubpassDescription() const noexcept {
            return VkSubpassDescription {
                .flags = flags,
                .pipelineBindPoint = pipelineBindPoint,
                .inputAttachmentCount = static_cast<uint32_t>(inputAttachments.size()),
                .pInputAttachments = inputAttachments.data(),
                .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
                .pColorAttachments = colorAttachments.data(),
                .pResolveAttachments = resolveAttachments.data(),
                .pDepthStencilAttachment = depthStencilAttachment ? depthStencilAttachment.ptr() : nullptr,
                .preserveAttachmentCount = static_cast<uint32_t>(preserveAttachments.size()),
                .pPreserveAttachments = preserveAttachments.data(),
            };
        }
    };

    struct RenderPass: public aui::noncopyable {
    public:


        RenderPass(Instance& instance, VkRenderPass handle): instance(instance), handle(handle) {} 
        RenderPass(Instance& instance, VkDevice device, const VkRenderPassCreateInfo& info): instance(instance), device(device), handle([&]{
            VkRenderPass pool;
            AUI_VK_THROW_ON_ERROR(instance.vkCreateRenderPass(device, &info, nullptr, &pool));
            return pool;
        }()) {} 

        RenderPass(RenderPass&& rhs) noexcept: instance(rhs.instance), device(rhs.device), handle(rhs.handle) {
            rhs.handle = 0;
        }

        ~RenderPass() {
            if (handle != 0) instance.vkDestroyRenderPass(device, handle, nullptr);
        }

        operator VkRenderPass() const noexcept {
            return handle;
        }

    private:
        Instance& instance;
        VkDevice device; 
        VkRenderPass handle; 
        
    };
}