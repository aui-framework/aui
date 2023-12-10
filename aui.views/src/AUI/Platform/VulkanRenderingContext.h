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

//
// Created by Alex2772 on 12/7/2021.
//

#pragma once


#include <AUI/Platform/CommonRenderingContext.h>
#include <vulkan/vulkan_core.h>
#include "ARenderingContextOptions.h"
#include "AUI/Util/APimpl.h"

class VulkanRenderer;
class VulkanRenderingContext: public CommonRenderingContext {
public:
    VulkanRenderingContext(const ARenderingContextOptions::Vulkan& config);

    void init(const Init& init) override;
    ~VulkanRenderingContext() override;

    AImage makeScreenshot() override;

    void destroyNativeWindow(ABaseWindow& window) override;
    void beginPaint(ABaseWindow& window) override;
    void endPaint(ABaseWindow& window) override;
    void beginResize(ABaseWindow& window) override;
    void endResize(ABaseWindow& window) override;

private:
    ARenderingContextOptions::Vulkan mConfig;
    _<VulkanRenderer> mRenderer;

    /**
     * @brief Image index that will be increased if the next image could be acquired.
     */
    uint32_t mImageIndex = 0;

    /**
     * @brief Frame index used for synchronization primitives.
     */
    uint32_t mCurrentFrame = 0;

    struct VulkanObjects;
    aui::fast_pimpl<AOptional<VulkanObjects>, sizeof(void*) * 64> mVulkan;

    const VulkanObjects& vulkan() const noexcept;

    static _<VulkanRenderer> ourRenderer();

    void recreateObjectsDueToResize();

#if AUI_PLATFORM_WIN
    static HGLRC ourHrc;
    static void makeCurrent(HDC hdc) noexcept;
#elif AUI_PLATFORM_LINUX
#elif AUI_PLATFORM_MACOS
    void* mContext;
#endif

};
