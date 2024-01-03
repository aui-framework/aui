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
        AString name;
        AColor color;
        std::chrono::high_resolution_clock::duration duration;
        AVector<Data> children;
    };

    using Datas = AVector<Data>;

#if AUI_PROFILING
    APerformanceSection(const char* name, AOptional<AColor> color = std::nullopt);
    ~APerformanceSection();
#else
    // expected to be optimized out
    APerformanceSection(const char* name, AOptional<AColor> color = std::nullopt) {}
    ~APerformanceSection() = default;
#endif

    void addSection(Data section) {
        mChildren << std::move(section);
    }

private:
#if AUI_PROFILING
    static APerformanceSection*& current() noexcept {
        static APerformanceSection* v = nullptr;
        return v;
    }

    AString mName;
    AColor mColor;
    std::chrono::high_resolution_clock::time_point mStart;
    AVector<Data> mChildren;

    APerformanceSection* mParent;

    static AColor generateColorFromName(const AString& name);
#endif
};
