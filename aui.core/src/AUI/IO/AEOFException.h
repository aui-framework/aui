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
#include "AIOException.h"

/**
 * @brief Thrown when stream has reached end (end of file).
 * @ingroup io
 */
class AEOFException: public AIOException
{
public:
	AEOFException() : AIOException("the stream has reached eof") {}

	virtual ~AEOFException() = default;
};
