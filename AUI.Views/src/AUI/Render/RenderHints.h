#pragma once

#include <glm/glm.hpp>
#include <AUI/Views.h>
#include <AUI/Common/AColor.h>
#include <AUI/GL/gl.h>
#include <AUI/Render/Render.h>

namespace RenderHints
{
	class PushMask
    {
    private:
        std::function<void()> mMaskRenderer;

    public:
        inline explicit PushMask(const std::function<void()>& maskRenderer):
                mMaskRenderer(maskRenderer)
        {
            pushMask(mMaskRenderer);
        }
        inline ~PushMask() {
            popMask(mMaskRenderer);
        }

        class Layer {
        private:
            uint8_t mPrevLayerValue;

        public:
            enum Direction {
                INCREASE = 1,
                DECREASE = -1,
            };
            explicit Layer(Direction direction, GLenum strencilComparsion = GL_EQUAL);
            ~Layer();

        };

        /**
         * \brief Увеличить стек маски без объекта. Используется для AView.
         * \note Не используйте эту функцию без крайней необходимости. Быстрее, проще, удобнее и безопаснее использовать
         *       <code>RenderHints::PushMask</code>.
         * \param maskRenderer функция-рендерер маски
         */
        API_AUI_VIEWS static void pushMask(const std::function<void()>& maskRenderer);

        /**
         * \brief Уменьшить стек маски без объекта. Используется для AView.
         * \note Не используйте эту функцию без крайней необходимости. Быстрее, проще, удобнее и безопаснее использовать
         *       <code>RenderHints::PushMask</code>.
         * \param maskRenderer функция-рендерер маски. Функция должна вернуть буфер трафарета к прежнему состоянию,
         *        то есть, отменить действие <code>RenderHints::PushMask::pushMask</code>
         */
        API_AUI_VIEWS static void popMask(const std::function<void()>& maskRenderer);
    };

	class PushMatrix
	{
	private:
		glm::mat4 mStored;
		
	public:
        inline PushMatrix() {
            mStored = Render::inst().getTransform();
		}
		inline ~PushMatrix() {
            Render::inst().setTransformForced(mStored);
		}
	};
	class PushColor
	{
	private:
		AColor mStored;
		
	public:
        inline PushColor() {
            mStored = Render::inst().getColor();
		}
		inline ~PushColor() {
            Render::inst().setColorForced(mStored);
		}
	};
	class PushState: PushColor, PushMatrix
	{
	};
};
