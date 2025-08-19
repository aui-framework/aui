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

/**
 * @defgroup profiling Profiling
 * @brief Performance profiling set of [devtools]
 * @details
 * Performance profiling capabilities are disabled by default. Compile with -DAUI_PROFILING=TRUE in order to enable.
 */

#include <chrono>
#include "AUI/Common/AColor.h"
#include "AUI/Common/AException.h"
#include "AUI/Performance/APerformanceSection.h"
#include "AUI/Traits/values.h"


/**
 * @brief Defines beginning and ending of window frame by RAII.
 * @ingroup core
 * @ingroup profiling 
 */
class API_AUI_CORE APerformanceFrame: public aui::noncopyable {
public:
    using Consumer = std::function<void(APerformanceSection::Datas sections)>;

#if AUI_PROFILING
    APerformanceFrame(Consumer consumer);
    ~APerformanceFrame();
#else
    APerformanceFrame() = default;
    ~APerformanceFrame() = default;
#endif

    static APerformanceFrame* current() {
#if AUI_PROFILING
        return currentStorage();
#else
        throw AException("AUI_PROFILING is disabled");
#endif
    }

    void addSection(APerformanceSection::Data section) {
        mSections << std::move(section);
    };

    [[nodiscard]]
    const APerformanceSection::Datas& sections() const noexcept {
        return mSections;
    };

private:
    APerformanceSection::Datas mSections;
    Consumer mConsumer;

    [[nodiscard]]
    static APerformanceFrame*& currentStorage() noexcept {
        thread_local APerformanceFrame* s = nullptr;
        return s;
    }
};