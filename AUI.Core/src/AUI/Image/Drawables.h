#pragma once
#include "IDrawable.h"
#include "AUI/Util/Cache.h"

class API_AUI_CORE Drawables: public Cache<IDrawable, Drawables>
{
public:
	_<IDrawable> load(const AString& key) override;
	~Drawables() override;
};
