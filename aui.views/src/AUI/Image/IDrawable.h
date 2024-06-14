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
        AOptional<glm::vec2> renderingSize;
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


    /**
     * @brief Creates a drawable from an url.
     * @param url url to create a drawable from
     * @return drawable instance or null
     * @details
     * If some kind of error occurs during loading the drawable, a log entry [Drawable] is outputted with detailed
     * description of an error. The reason is we don't want to crash our application if we didn't loaded some graphics,
     * which is usually pretty optional stuff. The user can still do his job with the application without fancy images.
     */
    API_AUI_VIEWS static _<IDrawable> fromUrl(const AUrl& url) noexcept;
};