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
#include "IInputStream.h"
#include "AUI/Common/ASignal.h"

/**
 * @brief Converts a basic input stream to an asynchronous input stream so it's read in a separate thread.
 * @ingroup io
 */
class API_AUI_CORE InputStreamAsync: public AObject
{
private:
	_<AThread> mReadThread;
	
public:
	InputStreamAsync(_<IInputStream> inputStream);
	
signals:
	emits<_<AByteBuffer>> read;
	emits<> finished;
};
