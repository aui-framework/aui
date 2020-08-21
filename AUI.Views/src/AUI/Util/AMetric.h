#pragma once

#include <AUI/Core.h>

class AString;

class API_AUI_CORE AMetric
{
public:
	enum Unit
	{
		T_UNKNOWN,
		T_PX,
		T_DP,
		T_PT,
	};
	
private:
	float mValue;
	Unit mUnit;
	
public:
	AMetric(float value, Unit unit);
	AMetric(const AString& text);

	[[nodiscard]] float getRawValue() const
	{
		return mValue;
	}

	[[nodiscard]] Unit getUnit() const
	{
		return mUnit;
	}

	[[nodiscard]] float getValuePx() const;
};


#pragma warning( disable : 4455 )
// ReSharper disable once CppUserDefinedLiteralSuffixDoesNotStartWithUnderscore
inline float operator"" dp(unsigned long long v)
{
	return AMetric(static_cast<float>(v), AMetric::T_DP).getValuePx();
}
