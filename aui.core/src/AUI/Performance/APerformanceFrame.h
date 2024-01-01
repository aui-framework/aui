// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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
#include "AUI/Traits/values.h"


/**
 * @brief Defines beginning and ending of window frame by RAII.
 * @ingroup core
 * @ingroup profiling 
 */
class API_AUI_CORE APerformanceFrame: public aui::noncopyable {
public:
    struct Section {
        AString name;
        AColor color;
        std::chrono::high_resolution_clock::duration duration;
    };
    using Sections = AVector<Section>;
    using Consumer = std::function<void(Sections sections)>;

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

    void addSection(Section section) {
        mSections << std::move(section);
    };

    [[nodiscard]]
    const Sections& sections() const noexcept {
        return mSections;
    };

private:
    Sections mSections;
    Consumer mConsumer;

    [[nodiscard]]
    static APerformanceFrame*& currentStorage() noexcept {
        static APerformanceFrame* s = nullptr;
        return s;
    }
};