/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#include "ARandom.h"
#include <ctime>

ARandom::ARandom()
{
	mRandom.seed(std::time(nullptr) ^ std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

int ARandom::nextInt()
{
	return std::uniform_int_distribution<int>()(mRandom);
}

float ARandom::nextFloat()
{
	return std::uniform_real_distribution<float>()(mRandom);
}

float ARandom::nextFloat(float min, float max)
{
	return std::uniform_real_distribution<float>(min, max)(mRandom);
}

glm::vec2 ARandom::nextVec2()
{
	return { nextFloat(), nextFloat() };
}

glm::vec2 ARandom::nextVec2(float min, float max)
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
		buf.put(reinterpret_cast<char*>(&c), 1);
	}
	return buf;
}

AUuid ARandom::nextUuid() {
	std::array<uint8_t, 16> array;
	for (auto& v : array) {
		v = std::uniform_int_distribution<unsigned>()(mRandom) % 0xff;
	}
	return AUuid(array);
}
