#pragma once

#include "AUI/Common/SharedPtr.h"
#include "AUI/Util/Singleton.h"
#include "AUI/Util/Manager.h"
#include "AUI/Render/AFont.h"

class API_AUI_VIEWS AFontManager : public Singleton<AFontManager>, public Manager<AFont> {
public:
	AFontManager();
	virtual ~AFontManager();
	AFontManager(const AFontManager&) = delete;
	_<FreeType> mFreeType;
	_<AFont> getDefault();
private:
	_<AFont> newItem(const AString& name) override;
	friend class AFont;
};
