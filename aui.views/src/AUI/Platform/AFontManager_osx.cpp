//
// Created by Alexey Titov on 29.11.2021.
//

#if AUI_PLATFORM_APPLE

#include "AFontManager.h"

AFontManager::AFontManager() :
        mFreeType(_new<FreeType>()) {
}

#endif