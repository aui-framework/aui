#include "ATextField.h"


ATextField::ATextField()
{
	AVIEW_CSS;
	addCssName(".input-field");
}

bool ATextField::isValidText(const AString& text) {
    return true;
}

ATextField::~ATextField() {

}
