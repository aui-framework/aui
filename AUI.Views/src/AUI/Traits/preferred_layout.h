#pragma once

#include <type_traits>
#include <AUI/Util/UIBuildingHelpers.h>

namespace aui::platform {
    namespace mobile {
        using preferred_layout = AVerticalLayout;
    }

    namespace desktop {
        using preferred_layout = AHorizontalLayout;
    }
}
