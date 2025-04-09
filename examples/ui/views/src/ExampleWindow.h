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

#include <AUI/Platform/AWindow.h>
#include "AUI/Thread/AAsyncHolder.h"

class API_AUI_AUDIO IAudioPlayer;

class ExampleWindow: public AWindow {
public:
	ExampleWindow();

    void onDragDrop(const ADragNDrop::DropEvent& event) override;

    bool onDragEnter(const ADragNDrop::EnterEvent& event) override;

private:
    ADeque<_<AWindow>> mWindows;
    AAsyncHolder mAsync;
    _<IAudioPlayer> mWavAudio;
    _<IAudioPlayer> mOggAudio;
};