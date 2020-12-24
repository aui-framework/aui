#pragma once
#include "ALabel.h"
#include "AViewContainer.h"

class API_AUI_VIEWS ACheckBox: public AViewContainer
{
private:
	_<ALabel> mText;
	bool mChecked = false;
	
public:
	ACheckBox();
	ACheckBox(const ::AString& text);
	virtual ~ACheckBox();

	void setText(const AString& text);


	[[nodiscard]] bool isChecked() const
	{
		return mChecked;
	}

	void setChecked(const bool checked)
	{
		mChecked = checked;
		emit customCssPropertyChanged();
	}

    bool consumesClick(const glm::ivec2& pos) override;

    void getCustomCssAttributes(AMap<AString, AVariant>& map) override;
    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;
signals:
	emits<bool> checked;
};
