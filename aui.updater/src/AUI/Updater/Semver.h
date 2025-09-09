#pragma once

#include <AUI/Common/AString.h>
namespace aui::updater {
    /**
     * @brief Semantic version.
     * @ingroup updater
     * @details
     * <!-- aui:parse_tests aui.updater/tests/VersionTest.cpp -->
     */
    struct API_AUI_UPDATER Semver {
        unsigned major{}, minor{}, patch{};


        /**
         * @brief Converts semver to a long int representation.
         */
        [[nodiscard]]
        uint64_t toLongInt() const noexcept {
            static constexpr auto STEP = 64 / 3;
            return ((major * STEP) + minor) * STEP + patch;
        }

        bool operator==(const Semver&) const = default;

        bool operator<(const Semver& rhs) const;
        bool operator>(const Semver& rhs) const;
        bool operator<=(const Semver& rhs) const;
        bool operator>=(const Semver& rhs) const;

        /**
         * @brief Parse semver from string.
         */
        static Semver fromString(const AString& version);
    };
}