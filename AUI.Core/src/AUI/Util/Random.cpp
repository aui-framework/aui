#include "Random.h"
#include <ctime>

Random::Random()
{
	mRandom.seed(std::time(nullptr));
}

int Random::nextInt()
{
	return std::uniform_int_distribution<int>()(mRandom);
}

float Random::nextFloat()
{
	return std::uniform_real_distribution<float>()(mRandom);
}

float Random::nextFloat(float min, float max)
{
	return std::uniform_real_distribution<float>(min, max)(mRandom);
}

glm::vec2 Random::nextVec2()
{
	return { nextFloat(), nextFloat() };
}

glm::vec2 Random::nextVec2(float min, float max)
{
	return { nextFloat(min, max), nextFloat(min, max) };
}

_<ByteBuffer> Random::nextBytes(unsigned count)
{
	auto buf = _new<ByteBuffer>();
	buf->reserve(count);
	for (unsigned i = 0; i < count; ++i)
	{
		unsigned char c = std::uniform_int_distribution<unsigned>()(mRandom) % 0xff;
		buf->put(reinterpret_cast<char*>(&c), 1);
	}
	return buf;
}
