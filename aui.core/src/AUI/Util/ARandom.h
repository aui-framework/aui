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
#include <random>
#include <glm/glm.hpp>
#include <AUI/Common/AUuid.h>
#include "AUI/Core.h"
#include "AUI/Common/AByteBuffer.h"

class API_AUI_CORE ARandom
{
private:
	std::default_random_engine mRandom;

public:
	ARandom();

	AUuid nextUuid() noexcept;
	int nextInt() noexcept;
	float nextFloat() noexcept;
	float nextFloat(float min, float max) noexcept;
	double nextDouble() noexcept;
	double nextDouble(double min, double max) noexcept;
	glm::vec2 nextVec2() noexcept;
	glm::vec2 nextVec2(float min, float max) noexcept;
	AByteBuffer nextBytes(unsigned count);
};
