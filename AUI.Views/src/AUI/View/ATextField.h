#pragma once

#include "AView.h"
#include "AAbstractTextField.h"

class API_AUI_VIEWS ATextField: public AAbstractTextField
{
public:
	ATextField();
	~ATextField() override;

protected:
	bool isValidText(const AString& text) override;
};
