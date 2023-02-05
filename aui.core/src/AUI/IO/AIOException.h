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