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

class VulkanRenderer;
class VulkanRenderingContext: public CommonRenderingContext {
public:
    VulkanRenderingContext(const ARenderingContextOptions::Vulkan& config) : mConfig(config) {}

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

    static _<VulkanRenderer> ourRenderer();

#if AUI_PLATFORM_WIN
    static HGLRC ourHrc;
    static void makeCurrent(HDC hdc) noexcept;
#elif AUI_PLATFORM_LINUX
#elif AUI_PLATFORM_MACOS
    void* mContext;
#endif

};
