#pragma once

#include <glm/glm.hpp>

/**
 * \brief An abstract image that determines itself how it is displayed. Essentially an abstraction from vector and
 *        raster graphics.
 */
class IDrawable
{
public:
	/**
	 * \brief Called when the image needs to be displayed. It is assumed that the renderer is already set to the
	 *        desired state (coordinates, color, etc.)
	 * \param size required image size. In common based on getSizeHint result
	 */
	virtual void draw(const glm::ivec2& size) = 0;

	
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
};