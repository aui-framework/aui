#pragma once

struct ABoxFields
{
	float left = 0.f;
	float right = 0.f;
	float top = 0.f;
	float bottom = 0.f;

	[[nodiscard]]
	float horizontal() const
	{
		return left + right;
	}
	
	[[nodiscard]]
	float vertical() const
	{
		return top + bottom;
	}
};
