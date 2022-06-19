/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <glm/glm.hpp>
#include <AUI/Views.h>
#include <AUI/Common/AColor.h>
#include <AUI/GL/gl.h>
#include <AUI/Render/Render.h>

namespace RenderHints
{
    /**
     * @brief Increases mask stack. Used by AView.
     * \note This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
     *       wrapper class instead.
     * \param maskRenderer function - mask renderer
     */
    template<typename Callable>
    static void pushMask(Callable&& maskRenderer) {
        Render::getRenderer()->pushMaskBefore();
        maskRenderer();
        Render::getRenderer()->pushMaskAfter();
    }

    /**
     * @brief Decreases mask stack. Used by AView.
     * \note This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
     *       wrapper class instead.
     * \param maskRenderer function - mask renderer
     */
    template<typename Callable>
    static void popMask(Callable&& maskRenderer) {
        Render::getRenderer()->popMaskBefore();
        maskRenderer();
        Render::getRenderer()->popMaskAfter();
    }

    template<typename Callable>
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
            mStored = Render::getTransform();
		}
		inline ~PushMatrix() {
            Render::setTransformForced(mStored);
		}
	};
	class PushColor
	{
	private:
		AColor mStored;
		
	public:
        inline PushColor() {
            mStored = Render::getColor();
		}
		inline ~PushColor() {
            Render::setColorForced(mStored);
		}
	};
	class PushState: PushColor, PushMatrix
	{
	};
};
