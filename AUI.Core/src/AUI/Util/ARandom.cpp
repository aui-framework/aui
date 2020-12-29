#include "ARandom.h"
#include <ctime>

ARandom::ARandom()
{
	mRandom.seed(std::time(nullptr));
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

_<AByteBuffer> ARandom::nextBytes(unsigned count)
{
	auto buf = _new<AByteBuffer>();
	buf->reserve(count);
	for (unsigned i = 0; i < count; ++i)
	{
		unsigned char c = std::uniform_int_distribution<unsigned>()(mRandom) % 0xff;
		buf->put(reinterpret_cast<char*>(&c), 1);
	}
	return buf;
}
