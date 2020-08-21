#pragma once
#include <functional>

class IShadingEffect
{
public:
	virtual void draw(const std::function<void()>& callback) = 0;
};
