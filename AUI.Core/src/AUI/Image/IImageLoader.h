#pragma once
#include "IDrawable.h"
#include "AUI/Common/SharedPtr.h"
#include "AImage.h"

class AByteBuffer;

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
	virtual bool matches(AByteBuffer& buffer) = 0;

	/**
	 * \brief The drawable (vector) image loader implementation.
	 * \note Called if and only if <code>matches</code> returned true.
	 * \return drawable (vector) image. Can be <code>nullptr</code> if <code>getRasterImage</code> implemented.
	 */
	virtual _<IDrawable> getDrawable(AByteBuffer& buffer) = 0;

	/**
	 * \brief The image loader implementation (raster).
	 * \note Called if and only if <code>matches</code> returned true.
	 * \return raster image. Can be <code>nullptr</code> if <code>getDrawable</code> implemented.
	 */
	virtual _<AImage> getRasterImage(AByteBuffer& buffer) = 0;
};

#include "AUI/Common/AByteBuffer.h"