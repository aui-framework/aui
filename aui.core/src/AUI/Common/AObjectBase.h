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

#include <AUI/Thread/AMutex.h>
#include <AUI/Traits/concepts.h>
#include <AUI/Traits/members.h>

#include "AUI/Common/ASet.h"
#include "AUI/Core.h"
#include "AUI/Traits/values.h"
#include "SharedPtrTypes.h"

class API_AUI_CORE AObjectBase: public aui::noncopyable {
    friend class AAbstractSignal;
public:
    AObjectBase() = default;

    AObjectBase(AObjectBase&& rhs) noexcept {
        AUI_ASSERTX(rhs.mSignals.empty(), "AObjectBase move is valid only if no signals connected to it");
    }
    ~AObjectBase() {
        clearSignals();
    }

    void clearSignals() noexcept;

private:
    ASet<AAbstractSignal*> mSignals;
};
