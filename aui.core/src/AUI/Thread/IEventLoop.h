// AUI Framework - Declarative UI toolkit for modern C++17
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

#include <AUI/Core.h>

class API_AUI_CORE IEventLoop
{
public:
	IEventLoop() {}
	virtual ~IEventLoop() {}

	/**
	 * @brief Notifies this IEventLoop that its thread got a new message to process.
	 */
	virtual void notifyProcessMessages() = 0;

	/**
	 * @brief Do message processing loop.
	 */
	virtual void loop() = 0;


    class API_AUI_CORE Handle {
	private:
        IEventLoop* mPrevEventLoop;
	    IEventLoop* mCurrentEventLoop;

	public:
        explicit Handle(IEventLoop* loop);
        ~Handle();
    };
};
