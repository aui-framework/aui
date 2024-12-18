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
#include <exception>

#include "AUI/Common/AException.h"

/**
 * @brief Exception caused by input/output stream.
 * @ingroup io
 */
class AIOException: public AException
{
public:
	AIOException()
	{
	}

	AIOException(const AString& message)
		: AException(message)
	{
	}
	virtual ~AIOException() = default;
};


class AFileNotFoundException: public AIOException {
public:
    using AIOException::AIOException;
};
class AAccessDeniedException: public AIOException {
public:
    using AIOException::AIOException;
};
class AResourceBusyException: public AIOException {
public:
    using AIOException::AIOException;
};
class ANoSpaceLeftException: public AIOException {
public:
    using AIOException::AIOException;
};