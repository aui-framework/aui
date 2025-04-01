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
