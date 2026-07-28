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

#include "ARandom.h"

#include <ctime>
#include <chrono>

ARandom::ARandom()
{
	mRandom.seed(std::time(nullptr) ^ std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

int ARandom::nextInt() noexcept
{
	return std::uniform_int_distribution<int>()(mRandom);
}

float ARandom::nextFloat() noexcept
{
	return std::uniform_real_distribution<float>()(mRandom);
}

float ARandom::nextFloat(float min, float max) noexcept
{
	return std::uniform_real_distribution<float>(min, max)(mRandom);
}
double ARandom::nextDouble() noexcept
{
	return std::uniform_real_distribution<double>()(mRandom);
}

double ARandom::nextDouble(double min, double max) noexcept
{
	return std::uniform_real_distribution<double>(min, max)(mRandom);
}

glm::vec2 ARandom::nextVec2() noexcept
{
	return { nextFloat(), nextFloat() };
}

glm::vec2 ARandom::nextVec2(float min, float max) noexcept
{
	return { nextFloat(min, max), nextFloat(min, max) };
}

AByteBuffer ARandom::nextBytes(unsigned count)
{
    AByteBuffer buf;
	buf.reserve(count);
	for (unsigned i = 0; i < count; ++i)
	{
		unsigned char c = std::uniform_int_distribution<unsigned>()(mRandom) % 0xff;
		buf.write(reinterpret_cast<char*>(&c), 1);
	}
	return buf;
}

AUuid ARandom::nextUuid() noexcept {
	std::array<uint8_t, 16> array;
	for (auto& v : array) {
		v = std::uniform_int_distribution<unsigned>()(mRandom) % 0xff;
	}
	return AUuid(array);
}
