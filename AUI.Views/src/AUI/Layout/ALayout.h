#pragma once
#include "AUI/Common/AObject.h"
#include "AUI/Common/ADeque.h"
#include "AUI/View/AView.h"
#include "AUI/Common/SharedPtr.h"

class AViewContainer;

class API_AUI_VIEWS ALayout: public AObject
{
protected:
	ADeque<_<AView>> mViews;
	
public:
	ALayout() = default;
	virtual ~ALayout() = default;
	virtual void onResize(int x, int y, int width, int height) = 0;
	virtual void addView(const _<AView>& view);
	virtual void removeView(const _<AView>& view);
	virtual int getMinimumWidth() = 0;
	virtual int getMinimumHeight() = 0;
	virtual void setSpacing(int spacing);
};
