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
#include "AUI/Common/SharedPtr.h"
#include "AImage.h"
#include "IImageFactory.h"

class AByteBuffer;
class IDrawable;

/**
 * \brief Class-loader of abstract images that can be displayed on the screen.
 */
class IImageLoader
{
public:
	/**
	 * \param buffer buffer with the raw image file contents.
	 * \return true, if this IImageLoader accepts image stored in this buffer
	 */
	virtual bool matches(const AByteBuffer& buffer) = 0;

	/**
	 * \brief The drawable (vector) image loader implementation.
	 * \note Called if and only if <code>matches</code> returned true.
	 * \return image factory. Can be <code>nullptr</code> if <code>getRasterImage</code> implemented.
	 */
	virtual _<IImageFactory> getImageFactory(const AByteBuffer& buffer) { return nullptr; };

	/**
	 * \brief The image loader implementation (raster).
	 * \note Called if and only if <code>matches</code> returned true.
	 * \return raster image. Can be <code>nullptr</code> if <code>getDrawable</code> implemented.
	 */
	virtual _<AImage> getRasterImage(const AByteBuffer& buffer) = 0;
};

#include "AUI/Common/AByteBuffer.h"