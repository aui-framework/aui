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

#include "InputStreamAsync.h"
#include <AUI/Common/AByteBuffer.h>

InputStreamAsync::InputStreamAsync(_<IInputStream> inputStream):
	mReadThread(_new<AThread>([&, inputStream]()
	{
		try {
			for (;;)
			{
                const size_t RESERVED = 0x1000;
				auto buffer = _new<AByteBuffer>(RESERVED);
				auto r = inputStream->read(buffer->end(), RESERVED);
                buffer->setSize(r);
				emit read(std::move(buffer));
			}
		} catch (...)
		{
			
		}
		emit finished();
	}))
{
	auto t = mReadThread;
	AThread::current() << [t] {
		t->start();
	};
}