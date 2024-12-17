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
