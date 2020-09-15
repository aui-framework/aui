#pragma once

#include <type_traits>

namespace aui::platform {
    namespace mobile {
        using is_vertical_layout_preferred = std::true_type;
    }

    namespace desktop {
        using is_vertical_layout_preferred = std::false_type;
    }

    namespace os_windows {
        using namespace desktop;
    }
    namespace os_unix {
        using namespace desktop;
    }
    namespace os_android {
        using namespace mobile;
    }

#if defined(_WIN32)
    namespace current = windows;
#elif defined(__ANDROID__)
    namespace current = os_android;
#else
    namespace current = os_unix;
#endif
}
