#pragma once
#include <cstdint>
#include <chrono>

namespace Util
{
	template<typename Function>
	uint64_t measureTimeInMillis(Function f)
	{
		using namespace std::chrono;
		const auto begin = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		f();
		const auto end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		return (end - begin).count();
	}
}
