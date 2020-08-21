#pragma once

#include "AView.h"
#include "AAbstractTextField.h"

class API_AUI_VIEWS ATextField: public AAbstractTextField
{
public:
	ATextField();
	virtual ~ATextField() = default;


protected:
	bool isValidText(const AString& text) override
	{
		return true;
	}
};
