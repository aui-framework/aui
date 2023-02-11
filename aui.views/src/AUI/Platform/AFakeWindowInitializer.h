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


#include "IRenderingContext.h"
#include "ABaseWindow.h"

/**
 * Fake window initializer which does not create any window. Applicable in UI tests
 */
class AFakeWindowInitializer: public IRenderingContext {
public:
    void init(const Init& init) override;

    ~AFakeWindowInitializer() override = default;

    void beginPaint(ABaseWindow& window) override;

    void endPaint(ABaseWindow& window) override;

    void beginResize(ABaseWindow& window) override;

    void destroyNativeWindow(ABaseWindow& window) override;
};


