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
#include <stdexcept>

/**
 * @brief Thrown when a parse error occur.
 * @ingroup xml
 */
class AXmlParseError: public std::runtime_error
{
public:
	AXmlParseError(const std::string& _Message)
		: runtime_error(_Message)
	{
	}

	AXmlParseError(const char* _Message)
		: runtime_error(_Message)
	{
	}
};
