#pragma once

#include <AUI/Common/AString.h>
#include "Matcher.h"

namespace By {
    API_AUI_UITESTS Matcher text(const AString& text);
    API_AUI_UITESTS Matcher name(const AString& name);
}


