//
// Created by alex2 on 07.11.2020.
//

#pragma once


#include <AUI/Common/AString.h>

/**
 * \brief Код языка в формате ISO 639-1, например, en-US или ru-RU
 */
class API_AUI_CORE ALanguageCode {
private:
    char mGroup[2];
    char mSubGroup[2];
public:
    ALanguageCode(const AString& str);
    ALanguageCode(const char* str): ALanguageCode(AString(str)) {}

    [[nodiscard]]
    AString toString() const {
        return AString(mGroup[0]) + mGroup[1] + "-" + mSubGroup[0] + mSubGroup[1];
    }

    [[nodiscard]]
    AString getGroup() const {
        return AString(mGroup, mGroup + 2);
    }

    [[nodiscard]]
    AString getSubGroup() const {
        return AString(mSubGroup, mSubGroup + 2);
    }
};


