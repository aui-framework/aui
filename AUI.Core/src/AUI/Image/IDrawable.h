#pragma once

#include <glm/glm.hpp>

/**
 * \brief Абстрактное изображение, которое само определяет способ его отображения.
 *	      По сути, абстракция от векторной и растровой графики.
 */
class IDrawable
{
public:
	/**
	 * \param size требуемый размер. Основывается на getSizeHint
	 * \brief Вызывается, когда изображение требуется отобразить. Предполагается,
	 *        что рендерер уже выставлен в нужное состояние (координаты, цвет и
	 *        т. д.)
	 */
	virtual void draw(const glm::ivec2& size) = 0;

	
	/**
	 * \return Размер хранимого изображения. Может быть проигнорирован рендерером.
	 *         Если размер неизвестен, то может быть {0, 0}
	 */
	virtual glm::ivec2 getSizeHint() = 0;


	/**
	 * \return true, если размер этого IDrawable зависит от DPI
	 */
	virtual bool isDpiDependent() const
	{
		return false;
	}
};