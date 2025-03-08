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

#include <AUI/Common/AException.h>


/**
 * @brief Indicates an evaluation loop.
 * @ingroup core
 * @details
 * AEvaluationLoopException is thrown by aui::lazy and therefore by APropertyPrecomputed.
 *
 * The condition under which AEvaluationLoopException is thrown is when there's an attempt to read from or access a
 * property during its own value evaluation. This creates an infinite evaluation loop, leading the system to throw this
 * exception to prevent potential stack overflow and unresponsiveness due to continuous evaluations.
 */
class AEvaluationLoopException: public AException {
public:
    AEvaluationLoopException(): AException("evaluation loop") {}
};

