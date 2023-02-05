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

#include "ARandom.h"
#include <ctime>

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
