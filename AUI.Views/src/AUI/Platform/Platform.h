#pragma once

#include "AUI/Views.h"

class AString;

namespace Platform
{
	API_AUI_VIEWS AString getFontPath(const AString& font);

	enum Sound
	{
		S_QUESTION,
		S_ASTERISK
	};

	/**
	 * \brief Play system sound asynchronously.
	 */
	API_AUI_VIEWS void playSystemSound(Sound s);

	API_AUI_VIEWS float getDpiRatio();
};
