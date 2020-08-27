#pragma once
#include "AView.h"

class API_AUI_VIEWS ASpacer: public AView
{
private:
    glm::ivec2 mMinimumSize = {20, 20};

public:
	ASpacer(int w = 4, int h = 4)
	{
		setExpanding({ w, h });
	}
	virtual ~ASpacer() = default;

    void setMinimumSize(const glm::ivec2& minimumSize) {
        mMinimumSize = minimumSize;
    }

    int getContentMinimumWidth() override;
    int getContentMinimumHeight() override;
};
