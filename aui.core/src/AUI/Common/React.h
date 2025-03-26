/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Common/AObjectBase.h>

/**
 * @brief Reactive expressions
 * @details
 * Implements reactive expressions.
 * @sa APropertyPrecomputed
 */
namespace aui::react {
struct DependencyObserver : AObjectBase {
public:
    virtual void invalidate() = 0;
};

struct API_AUI_CORE DependencyObserverRegistrar {
    explicit DependencyObserverRegistrar(DependencyObserver& observer);
    ~DependencyObserverRegistrar();

    private:
    DependencyObserver* mPrevObserver;
};

/**
 * @brief Adds observer to the specified signal, if called inside a reactive expression evaluation.
 */
API_AUI_CORE void addDependency(const AAbstractSignal& signal);

}   // namespace aui::react
