// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
#include "AUI/Common/SharedPtr.h"
#include "AImage.h"
#include "IImageFactory.h"
#include <AUI/Common/AByteBufferView.h>

class AByteBuffer;
class IDrawable;

/**
 * @brief Class-loader of abstract images that can be displayed on the screen.
 */
class IImageLoader
{
public:
	/**
	 * @param buffer buffer with the raw image file contents.
	 * @return true, if this IImageLoader accepts image stored in this buffer
	 */
	virtual bool matches(AByteBufferView buffer) = 0;

	/**
	 * @brief The drawable (vector) image loader implementation.
	 * @note Called if and only if <code>matches</code> returned true.
	 * @return image factory. Can be <code>nullptr</code> if <code>getRasterImage</code> implemented.
	 */
	virtual _<IImageFactory> getImageFactory(AByteBufferView buffer) { return nullptr; };

	/**
	 * @brief The image loader implementation (raster).
	 * @note Called if and only if <code>matches</code> returned true.
	 * @return raster image. Can be <code>nullptr</code> if <code>getDrawable</code> implemented.
	 */
	virtual _<AImage> getRasterImage(AByteBufferView buffer) = 0;
};

#include "AUI/Common/AByteBuffer.h"