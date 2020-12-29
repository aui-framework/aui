#pragma once

#include <AUI/Core.h>

class AString;

class API_AUI_VIEWS AMetric
{
public:
	enum Unit
	{
		T_UNKNOWN,
		T_PX,
		T_EM,
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


inline float operator"" _dp(unsigned long long v)
{
	return AMetric(static_cast<float>(v), AMetric::T_EM).getValuePx();
}
inline float operator"" _pt(unsigned long long v)
{
	return AMetric(static_cast<float>(v), AMetric::T_PT).getValuePx();
}
