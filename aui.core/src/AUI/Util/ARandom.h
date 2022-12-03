// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
