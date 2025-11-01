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

#include <AUI/Common/AObjectBase.h>

/**
 * @defgroup reactive Reactive expressions
 * @details
 * Reactive expressions are a way to create dynamic values that automatically update when their dependencies change.
 * They're particularly useful for creating computed properties that depend on other properties.
 *
 * ```cpp
 * APropertyPrecomputed<int> sum = [&] {
 *   return property1 + property2;  // automatically tracks dependencies
 * };
 * ```
 *
 * This creates a readonly property that computes its value based on other properties. Dependencies are automatically
 * tracked when the expression accesses other properties. The value is recomputed lazily when accessed after
 * dependencies change.
 */

/**
 * @brief Reactive expressions namespace.
 */
namespace aui::react {
struct DependencyObserver : AObjectBase {
    friend struct API_AUI_CORE DependencyObserverScope;

public:
    virtual void invalidate() = 0;

private:
    AVector<AAbstractSignal::AutoDestroyedConnection> mObserverConnections;
};

struct API_AUI_CORE DependencyObserverScope {
    explicit DependencyObserverScope(DependencyObserver* observer);
    ~DependencyObserverScope();

    /**
     * @brief Adds observer to the specified signal, if called inside a reactive expression evaluation.
     */
    static void addDependency(const AAbstractSignal& signal);

private:
    DependencyObserver* mPrevObserver;
};
}   // namespace aui::react
