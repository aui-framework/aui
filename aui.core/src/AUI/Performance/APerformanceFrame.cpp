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

#if AUI_PROFILING

#include "AUI/Performance/APerformanceFrame.h"

APerformanceFrame::APerformanceFrame(Consumer consumer): mConsumer(std::move(consumer)) {
    AUI_ASSERTX(currentStorage() == nullptr, "there are could not be 2 APerformance frames at the same time");
    currentStorage() = this;
}

APerformanceFrame::~APerformanceFrame() {
    AUI_ASSERT(currentStorage() == this);
    currentStorage() = nullptr;

    mConsumer(std::move(mSections));
}

#endif