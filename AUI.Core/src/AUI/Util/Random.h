#pragma once
#include <random>
#include <glm/glm.hpp>
#include "AUI/Core.h"
#include "AUI/Common/ByteBuffer.h"

class API_AUI_CORE Random
{
private:
	std::default_random_engine mRandom;

public:
	Random();

	int nextInt();
	float nextFloat();
	float nextFloat(float min, float max);
	glm::vec2 nextVec2();
	glm::vec2 nextVec2(float min, float max);

	_<ByteBuffer> nextBytes(unsigned count);
};
