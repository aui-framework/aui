/**
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
#include <AUI/Util/Cache.h>
#include <AUI/Url/AUrl.h>
#include <AUI/Enum/Repeat.h>
#include <AUI/Enum/ImageRendering.h>


/**
 * \brief An abstract image that determines itself how it is displayed. Essentially an abstraction from vector and
 *        raster graphics.
 */
class IDrawable
{
    friend class AImageLoaderRegistry;

    class Cache;
    friend class ::Cache<IDrawable, Cache, AUrl>;
    friend class AImageLoaderRegistry;
    class Cache: public ::Cache<IDrawable, Cache, AUrl> {
    public:
        static Cache& inst();
    protected:
        _<IDrawable> load(const AUrl& key) override;
    };

public:

    struct Params {
        glm::vec2 offset = {0.f, 0.f};
        glm::vec2 size;
        std::optional<glm::vec2> cropUvTopLeft;
        std::optional<glm::vec2> cropUvBottomRight;

        Repeat repeat = Repeat::NONE;
        ImageRendering imageRendering;
    };

	/**
	 * \brief Called when the image needs to be displayed. It is assumed that the renderer is already set to the
	 *        desired state (coordinates, color, etc.)
	 */
	virtual void draw(const Params& params) = 0;

	
	/**
	 * \return Size of the stored image. Can be ignored by the renderer. If the size is unknown, it can be {0, 0}
	 */
	virtual glm::ivec2 getSizeHint() = 0;


	/**
	 * \return true if the size hint of this drawable needs to be multiplied by the DPI ratio
	 */
	virtual bool isDpiDependent() const
	{
		return false;
	}


    API_AUI_VIEWS static _<IDrawable> fromUrl(const AUrl& url);
};