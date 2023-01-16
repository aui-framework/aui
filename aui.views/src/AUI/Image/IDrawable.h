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
#include <AUI/Util/Cache.h>
#include <AUI/Url/AUrl.h>
#include <AUI/Enum/Repeat.h>
#include <AUI/Enum/ImageRendering.h>
#include <AUI/Image/AImage.h>


/**
 * @brief An abstract image that determines itself how it is displayed. Essentially an abstraction from vector and
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
        AOptional<glm::vec2> cropUvTopLeft;
        AOptional<glm::vec2> cropUvBottomRight;

        Repeat repeat = Repeat::NONE;
        ImageRendering imageRendering;
    };

    /**
     * @brief Converts possibly vector drawable to a raster image.
     * @param imageSize image size
     * @return rasterized image
     * @details
     * TODO Unimplemented for all drawables except raster image and svg
     */
    virtual AImage rasterize(glm::ivec2 imageSize);

	/**
	 * @brief Called when the image needs to be displayed. It is assumed that the renderer is already set to the
	 *        desired state (coordinates, color, etc.)
	 */
	virtual void draw(const Params& params) = 0;

	
	/**
	 * @return Size of the stored image. Can be ignored by the renderer. If the size is unknown, it can be {0, 0}
	 */
	virtual glm::ivec2 getSizeHint() = 0;


	/**
	 * @return true if the size hint of this drawable needs to be multiplied by the DPI ratio
	 */
	virtual bool isDpiDependent() const
	{
		return false;
	}


    API_AUI_VIEWS static _<IDrawable> fromUrl(const AUrl& url);
};