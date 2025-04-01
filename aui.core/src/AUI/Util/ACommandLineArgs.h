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

#include <AUI/Common/AStringVector.h>

/**
 * @brief Simple command line arguments parser.
 * @ingroup core
 * @details
 * @see aui::args()
 */
class API_AUI_CORE ACommandLineArgs: public AStringVector {
public:
    using AStringVector::AStringVector;

    /**
     * @brief Finds --key=value style argument.
     * @param flagName key name (without 2 dashes)
     * @return value, if presented
     */
    AOptional<AString> value(const AString& flagName) const;
};