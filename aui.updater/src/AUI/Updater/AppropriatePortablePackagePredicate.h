#pragma once

#include <AUI/Common/AObject.h>

namespace aui::updater {

    /**
     * @brief Determines whether the passed package name is a portable package that matches current arch and platform.
     * @ingroup updater
     * @details
     * <!-- aui:parse_tests aui.updater/tests/AppropriatePredicateTest.cpp -->
     */
    struct API_AUI_UPDATER AppropriatePortablePackagePredicate {
        /**
         * @brief Package qualifier which is expected to be present in package name.
         * @details
         * If qualifier is empty, an AUI's package qualifier which is used as default name for portable packages is
         * used.
         *
         * Since AUI is only capable of unpacking ZIPs, this predicate checks for ".zip" extension.
         *
         * Implemented as AString::contains with an additional check that "_" does not goes after, to distinguish
         * `x86_64` and `x86`.
         *
         * Linux x86_64 -> `linux-x86_64`
         * Windows arm64 -> `windows-arm64`
         *
         * If you wish to publish portable packages with different qualifiers, you must specify it to this variable.
         * As AUI's package name, it's convenient to evaluate the package name in CMake and pass it to your target
         * with `target_compile_definitions`.
         */
        AString qualifier{};

        [[nodiscard]]
        bool operator()(const AString& packageName) const noexcept;

        AString getQualifierDebug() const;
        const AString& getQualifier() const;
    };
}