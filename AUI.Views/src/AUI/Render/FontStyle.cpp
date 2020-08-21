#include "FontStyle.h"

#include "AFontManager.h"

size_t FontStyle::getWidth(const AString& text)
{
	if (!font)
		font = AFontManager::instance().getDefault();

	return font->length(text, size, fontRendering);
}
