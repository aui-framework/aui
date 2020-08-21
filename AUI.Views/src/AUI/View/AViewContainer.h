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


	void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor)
	override;
public:
	AViewContainer();
	virtual ~AViewContainer();
	void addView(_<AView> view);
	void removeView(_<AView> view);

	void render() override;


	void onMouseEnter() override;
	void onMouseMove(glm::ivec2 pos) override;
	void onMouseLeave() override;



	int getContentMinimumWidth() override;
	int getContentMinimumHeight() override;
	
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
	void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) override;
	void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;
	
	void setSize(int width, int height) override;

	/**
	 * \brief выставить компоновщик для этого контейнера. УНИЧТОЖАЕТ СТАРЫЙ
	 *        КОМПОНОВЩИК ВМЕСТЕ СО ВСЕМИ ЕГО ЭЛЕМЕНТАМИ!!
	 */
	void setLayout(_<ALayout> layout);
	_<ALayout> getLayout() const;

	_<AView> getViewAt(glm::ivec2 pos, bool ignoreGone = true);
	_<AView> getViewAtRecusrive(glm::ivec2 pos);
	
	void updateLayout();


	const AVector<_<AView>>& getViews() const
	{
		return mViews;
	}
};
