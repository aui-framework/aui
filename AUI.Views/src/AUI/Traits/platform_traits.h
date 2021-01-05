#pragma once

#include <type_traits>

namespace aui::traits {
    struct android_platform {
        using is_vertical_preferred = std::true_type;
    };
    struct windows_platform {
        using is_vertical_preferred = std::false_type;
    };
    struct linux_platform {
        using is_vertical_preferred = std::false_type;
    };

    using current_platform = android_platform;
}
