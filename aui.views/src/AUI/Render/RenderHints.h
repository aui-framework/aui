// AUI Framework - Declarative UI toolkit for modern C++17
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

#include <glm/glm.hpp>
#include <AUI/Views.h>
#include <AUI/Common/AColor.h>
#include <AUI/GL/gl.h>
#include <AUI/Render/Render.h>

namespace RenderHints
{
    /**
     * @brief Increases mask stack. Used by AView.
     * @note This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
     *       wrapper class instead.
     * @param maskRenderer function - mask renderer
     */
    template<typename Callable>
    static void pushMask(Callable&& maskRenderer) {
        Render::getRenderer()->pushMaskBefore();
        maskRenderer();
        Render::getRenderer()->pushMaskAfter();
    }

    /**
     * @brief Decreases mask stack. Used by AView.
     * @note This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
     *       wrapper class instead.
     * @param maskRenderer function - mask renderer
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
