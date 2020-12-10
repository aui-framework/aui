#pragma once

#include <AUI/Views.h>
#include "AUI/Common/SharedPtr.h"
#include <glm/glm.hpp>
#include "AUI/Layout/ALayout.h"
#include "AUI/Common/AVector.h"
#include "AUI/Render/Render.h"
#include "AUI/Render/RenderHints.h"

class API_AUI_VIEWS AViewContainer: public AView
{
private:
	_<ALayout> mLayout;
    bool mSizeSet = false;
    bool mHasBackground = false;
	glm::ivec2 mPreviousSize = mSize;

protected:
	AVector<_<AView>> mViews;
	
	void drawView(const _<AView>& view);

	template <typename Iterator>
	void drawViews(Iterator begin, Iterator end)
	{
		for (auto i = begin; i != end; ++i)
		{
			drawView(*i);
		}
	}

    void recompileCSS() override;

    void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor)
	override;

    /**
     * \brief Обновляет макет родительского AView, если размер этого элемента был изменён.
     */
    void updateParentsLayoutIfNecessary();

public:
	AViewContainer();
	virtual ~AViewContainer();
	void addView(_<AView> view);
	void removeView(_<AView> view);
	void removeAllViews();

	void render() override;


	void onMouseEnter() override;
	void onMouseMove(glm::ivec2 pos) override;
	void onMouseLeave() override;



	int getContentMinimumWidth() override;
	int getContentMinimumHeight() override;
	
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
	void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) override;
	void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    void onMouseWheel(glm::ivec2 pos, int delta) override;

    bool consumesClick(const glm::ivec2& pos) override;

    void setSize(int width, int height) override;

	/**
	 * \brief выставить компоновщик для этого контейнера. УНИЧТОЖАЕТ СТАРЫЙ
	 *        КОМПОНОВЩИК ВМЕСТЕ СО ВСЕМИ ЕГО ЭЛЕМЕНТАМИ!!
	 */
	void setLayout(_<ALayout> layout);
	_<ALayout> getLayout() const;

	_<AView> getViewAt(glm::ivec2 pos, bool ignoreGone = true);
	_<AView> getViewAtRecursive(glm::ivec2 pos);

	template<typename T>
	_<T> getViewAtRecursiveOf(glm::ivec2 pos, bool ignoreGone = true) {
		for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
		{
			auto view = *it;
			auto targetPos = pos - view->getPosition();

			if (targetPos.x >= 0 && targetPos.y >= 0 && targetPos.x < view->getSize().x && targetPos.y < view->getSize().y)
			{
				if (!ignoreGone || view->getVisibility() != V_GONE) {
					if (auto applicable = _cast<T>(view))
						return applicable;
					if (auto container = _cast<AViewContainer>(view)) {
						if (auto applicable = container->getViewAtRecursiveOf<T>(targetPos, ignoreGone)) {
							return applicable;
						}
					}
				}
			}
		}
        return nullptr;
	}

	virtual void updateLayout();

public:

    const AVector<_<AView>>& getViews() const
	{
		return mViews;
	}
};
