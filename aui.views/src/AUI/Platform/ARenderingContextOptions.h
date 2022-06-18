#pragma once

#include <AUI/Common/AVector.h>
#include <variant>
#include "AUI/Reflect/AEnumerate.h"



AUI_ENUM_FLAG(ARenderContextFlags) {
    DEFAULT = 0b0,

    /**
     * @brief Controls AUI's smooth window resize feature. If set to false, window's would resize dirty, however,
     * performance is increased by 10-50% especially in games.
     */
    NO_SMOOTH = 0b1,

    /**
     * @brief Disables vsync, causing the machine render as frequently as possible.
     */
    NO_VSYNC = 0b10,
};

struct API_AUI_VIEWS ARenderingContextOptions {
public:

    struct DirectX11 {
        int majorVersion;
    };

    struct OpenGL {
        int majorVersion = 2, minorVersion = 0;

        enum class Profile {
            CORE,
            COMPAT
        } profile = Profile::CORE;
    };

    struct Software {};

    using InitializationVariant =  std::variant<DirectX11,
            OpenGL,
            Software>;

    AVector<InitializationVariant> initializationOrder;

    ARenderContextFlags flags = ARenderContextFlags::DEFAULT;

    static void set(ARenderingContextOptions options) {
        inst() = std::move(options);
    }
    static const ARenderingContextOptions& get() noexcept {
        return inst();
    }

private:
    static ARenderingContextOptions& inst();
};

AUI_ENUM_VALUES(ARenderingContextOptions::OpenGL::Profile,
                ARenderingContextOptions::OpenGL::Profile::CORE,
                ARenderingContextOptions::OpenGL::Profile::COMPAT)
