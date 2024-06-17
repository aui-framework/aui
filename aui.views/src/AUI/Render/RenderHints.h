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

#include <glm/glm.hpp>
#include <AUI/Views.h>
#include <AUI/Common/AColor.h>
#include <AUI/GL/gl.h>
#include "ARender.h"

namespace RenderHints
{
    /**
     * @brief Increases mask stack. Used by AView.
     * @note This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
     *       wrapper class instead.
     * @param maskRenderer function - mask renderer
     */
    template<aui::invocable Callable>
    static void pushMask(Callable&& maskRenderer) {
        ARender::getRenderer()->pushMaskBefore();
        maskRenderer();
        ARender::getRenderer()->pushMaskAfter();
    }

    /**
     * @brief Decreases mask stack. Used by AView.
     * @note This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
     *       wrapper class instead.
     * @param maskRenderer function - mask renderer
     */
    template<aui::invocable Callable>
    static void popMask(Callable&& maskRenderer) {
        ARender::getRenderer()->popMaskBefore();
        maskRenderer();
        ARender::getRenderer()->popMaskAfter();
    }

    template<aui::invocable Callable>
    class PushMask
    {
    private:
        Callable mMaskRenderer;

    public:
        inline explicit PushMask(Callable&& maskRenderer):
                mMaskRenderer(std::forward<Callable>(maskRenderer))
        {
            pushMask(std::forward<Callable>(mMaskRenderer));
        }
        inline ~PushMask() {
            popMask(std::forward<Callable>(mMaskRenderer));
        }
    };

    class PushMatrix
	{
	private:
		glm::mat4 mStored;
		
	public:
        inline PushMatrix() {
            mStored = ARender::getTransform();
		}
		inline ~PushMatrix() {
            ARender::setTransformForced(mStored);
		}
	};
	class PushColor
	{
	private:
		AColor mStored;
		
	public:
        inline PushColor() {
            mStored = ARender::getColor();
		}
		inline ~PushColor() {
            ARender::setColorForced(mStored);
		}
	};
	class PushState: PushColor, PushMatrix
	{
	};
};
