#pragma once
#include "IDrawable.h"
#include "AUI/Common/SharedPtr.h"
#include "AImage.h"

class AByteBuffer;

/**
 * \brief Класс-загрузчик абстрактных изображений, которые можно отобразить на экране.
 */
class IImageLoader
{
public:
	/**
	 * \return true, если этот загрузчик может загрузить изображение, находящееся в
	 *	       этом буфере.
	 */
	virtual bool matches(_<AByteBuffer> buffer) = 0;

	/**
	 * \brief Вызывается тогда и только тогда, когда matches вернёт true.
	 *        Непосредственно загрузчик изображения (векторный).
	 * \return векторное изображение (nullable)
	 */
	virtual _<IDrawable> getDrawable(_<AByteBuffer> buffer) = 0;

	/**
	 * \brief Вызывается тогда и только тогда, когда matches вернёт true.
	 *        Непосредственно загрузчик изображения (растровый)
	 * \return растровое изображение (nullable)
	 */
	virtual _<AImage> getRasterImage(_<AByteBuffer> buffer) = 0;
};

#include "AUI/Common/AByteBuffer.h"