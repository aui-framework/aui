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
	AMetric(float value, Unit unit = T_PX);
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

	operator float() const {
	    return getValuePx();
	}
};


inline AMetric operator"" _dp(unsigned long long v)
{
	return AMetric(static_cast<float>(v), AMetric::T_EM);
}
inline AMetric operator"" _pt(unsigned long long v)
{
	return AMetric(static_cast<float>(v), AMetric::T_PT);
}
