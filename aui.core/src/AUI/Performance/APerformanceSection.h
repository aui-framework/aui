/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <chrono>
#include <optional>

#include "AUI/Common/AColor.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/AVector.h"

/**
 * @brief Defines performance profiling named (and colored) span within RAII range.
 * @ingroup core
 * @ingroup profiling
 * @sa profiling
 */
class API_AUI_CORE APerformanceSection {
public:
    struct Data {
        /**
         * @brief Name of the section. Expected to be alive whole program lifetime.
         */
        const char* name;

        /**
         * @brief Color of the section.
         */
        AColor color;
        std::string verboseInfo;
        std::chrono::high_resolution_clock::duration duration = std::chrono::high_resolution_clock::duration(0);
        AVector<Data> children;
    };

    using Datas = AVector<Data>;

#if AUI_PROFILING
    /**
     * @brief Defines performance profiling named (and colored) span within RAII range.
     * @param name name of the section. Since it is a C-style string, it must be alive during the whole lifetime of the
     *             program. Typically, you would want to use string literals. C-style string is used to avoid
     *             performance overhead.
     * @param color color of the section. If nullopt, it would be generated from name.
     * @param verboseInfo extra usefull information that displayed in tree view in paused mode.
     */
    APerformanceSection(const char* name, AOptional<AColor> color = std::nullopt, std::string verboseInfo = {});
    ~APerformanceSection();

    void addSection(Data section) {
        mChildren << std::move(section);
    }

#else
    // expected to be optimized out

    /**
     * @brief Defines performance profiling named (and colored) span within RAII range.
     * @param name name of the section. Since it is a C-style string, it must be alive during the whole lifetime of the
     *             program. Typically, you would want to use string literals. C-style string is used to avoid
     *             performance overhead.
     * @param color color of the section. If nullopt, it would be generated from name.
     * @param verboseInfo extra usefull information that displayed in tree view in paused mode.
     */
    APerformanceSection(const char* name, AOptional<AColor> color = std::nullopt, std::string verboseInfo = {}) {}
    ~APerformanceSection() = default;
#endif

private:
#if AUI_PROFILING
    static APerformanceSection*& current() noexcept {
        thread_local APerformanceSection* v = nullptr;
        return v;
    }

    const char* mName;
    AColor mColor;
    std::string mVerboseInfo;
    std::chrono::high_resolution_clock::time_point mStart;
    AVector<Data> mChildren;

    APerformanceSection* mParent;

    static AColor generateColorFromName(const char* name);
#endif
};
