#pragma once
#include "IDrawable.h"
#include "AUI/Common/SharedPtr.h"

class ByteBuffer;

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
	virtual bool matches(_<ByteBuffer> buffer) = 0;

	/**
	 * \brief Вызывается тогда и только тогда, когда matches вернёт true.
	 *        Непосредственно загрузчик изображения.
	 * 
	 */
	virtual _<IDrawable> getDrawable(_<ByteBuffer> buffer) = 0;
};
