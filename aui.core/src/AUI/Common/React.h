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
 * @defgroup react Reactive expressions
 * @details
 * Implements reactive expressions.
 */

/**
 * @brief Reactive expressions namespace.
 */
namespace aui::react {
struct DependencyObserver : AObjectBase {
    friend struct API_AUI_CORE DependencyObserverRegistrar;

public:
    virtual void invalidate() = 0;

private:
    AVector<AAbstractSignal::AutoDestroyedConnection> mObserverConnections;
};

struct API_AUI_CORE DependencyObserverRegistrar {
    explicit DependencyObserverRegistrar(DependencyObserver& observer);
    ~DependencyObserverRegistrar();

    /**
     * @brief Adds observer to the specified signal, if called inside a reactive expression evaluation.
     */
    static void addDependency(const AAbstractSignal& signal);

private:
    DependencyObserver* mPrevObserver;
};

}   // namespace aui::react
