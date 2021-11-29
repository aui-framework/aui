//
// Created by Alexey Titov on 29.11.2021.
//

#include <AUI/Platform/AFontManager.h>


AFontManager::AFontManager() :
        mFreeType(_new<FreeType>()) {
}