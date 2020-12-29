#pragma once
#include <AUI/Image/IDrawable.h>
#include <AUI/Image/AImage.h>
#include "AUI/Util/Cache.h"

class API_AUI_VIEWS Drawables: public Cache<IDrawable, Drawables>
{
public:
	_<IDrawable> load(const AString& key) override;
	~Drawables();

    static Drawables& inst();
};
