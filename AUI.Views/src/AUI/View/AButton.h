#pragma once
#include "AView.h"
#include "AUI/Common/AString.h"
#include "AUI/Render/Render.h"
#include "ALabel.h"

class API_AUI_VIEWS AButton: public ALabel
{	
public:
	AButton();
	explicit AButton(const AString& text);
	virtual ~AButton() = default;

	void setDefault();
	
	void getCustomCssAttributes(AMap<AString, AVariant>& map) override;
	
signals:
	emits<bool> defaultState;
	emits<> becameDefault;
	emits<> noLongerDefault;

private:
	Watchable<bool> mDefault = Watchable<bool>(defaultState, becameDefault, noLongerDefault);
};
