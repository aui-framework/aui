// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

/**
 * @defgroup profiling Profiling
 * @brief Performance profiling set of tools
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