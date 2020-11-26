#pragma once

#include <AUI/IO/APath.h>
#include "AVariant.h"


template<>
struct AVariantHelper<std::nullptr_t> : AVariantHelperBase
{
public:
	AVariantHelper(std::nullptr_t) {}

	~AVariantHelper() override = default;
	int toInt() override
	{
		return 0;
	}
	unsigned toUInt() override
	{
		return 0;
	}
	float toFloat() override
	{
		return 0.f;
	}
	double toDouble() override
	{
		return 0.0;
	}
	bool toBool() override
	{
		return false;
	}
	AString toString() override
	{
		return "null";
	}

	size_t hash() override
	{
		return 228;
	}

    AVariantType::Type getType() override {
        return AVariantType::AV_NULL;
    }
};

template <typename T>
struct AVariantHelperNumeric : AVariantHelperBase
{
private:
	T mStored;

public:
	AVariantHelperNumeric(T stored)
		: mStored(stored)
	{
	}
	
	~AVariantHelperNumeric() override = default;
	int toInt() override
	{
		return static_cast<int>(mStored);
	}
	unsigned toUInt() override
	{
		return static_cast<unsigned>(mStored);
	}
	float toFloat() override
	{
		return static_cast<float>(mStored);
	}
	double toDouble() override
	{
		return static_cast<double>(mStored);
	}
	bool toBool() override
	{
		return static_cast<bool>(mStored);
	}
	AString toString() override
	{
		return AString::number(static_cast<T>(mStored));
	}

	size_t hash() override
	{
		return std::hash<T>()(mStored);
	}

    AVariantType::Type getType() override {
        return AVariantType::getTypeOf<T>();
    }

    T& getData() {
	    return mStored;
	}
};


template<> struct AVariantHelper<int>: AVariantHelperNumeric<int>
{
	AVariantHelper(int stored)
		: AVariantHelperNumeric<int>(stored)
	{
	}
};
template<> struct AVariantHelper<unsigned>: AVariantHelperNumeric<unsigned>
{
	AVariantHelper(unsigned stored)
		: AVariantHelperNumeric<unsigned>(stored)
	{
	}
};
template<> struct AVariantHelper<float>: AVariantHelperNumeric<float>
{
	AVariantHelper(float stored)
		: AVariantHelperNumeric<float>(stored)
	{
	}
};
template<> struct AVariantHelper<double>: AVariantHelperNumeric<double>
{
	AVariantHelper(double stored)
		: AVariantHelperNumeric<double>(stored)
	{
	}
};
template<> struct AVariantHelper<bool>: AVariantHelperNumeric<bool>
{
	AVariantHelper(bool stored)
		: AVariantHelperNumeric<bool>(stored)
	{
	}
};

template<>
struct AVariantHelper<AString>: AVariantHelperBase
{
private:
	AString mString;

public:
	AVariantHelper(const AString& string):
		mString(string)
	{
	}

	~AVariantHelper() override = default;
	int toInt() override
	{
		return mString.toInt();
	}
	unsigned toUInt() override
	{
		return mString.toInt();
	}
	float toFloat() override
	{
		return mString.toFloat();
	}
	double toDouble() override
	{
		return mString.toDouble();
	}
	bool toBool() override
	{
		return mString.toBool();
	}
	AString toString() override
	{
		return mString;
	}

	size_t hash() override
	{
		return std::hash<std::string>()(mString.toStdString());
	}

    AVariantType::Type getType() override {
        return AVariantType::AV_STRING;
    }
};

template<>
struct AVariantHelper<APath>: AVariantHelper<AString> {
    AVariantHelper(const APath& string):
            AVariantHelper<AString>(string)
    {
    }
};


template<>
struct AVariantHelper<std::string> : AVariantHelperBase
{
private:
	std::string mString;

public:
	AVariantHelper(const std::string& string) :
		mString(string)
	{
	}
	AVariantHelper() = default;

	~AVariantHelper() override = default;
	int toInt() override
	{
		try {
			return std::stoi(mString);
		} catch (...)
		{
			return 0;
		}
	}
	unsigned toUInt() override
	{
		try {
			return std::stoul(mString);
		}
		catch (...)
		{
			return 0;
		}
	}
	float toFloat() override
	{
		try {
			return std::stof(mString);
		}
		catch (...)
		{
			return 0;
		}
	}
	double toDouble() override
	{
		try {
			return std::stod(mString);
		}
		catch (...)
		{
			return 0;
		}
	}
	bool toBool() override
	{
		return mString == "true";
	}
	AString toString() override
	{
		return mString.c_str();
	}

	size_t hash() override
	{
		return std::hash<std::string>()(mString);
	}

	std::string& getData()
	{
		return mString;
	}

    AVariantType::Type getType() override {
        return AVariantType::AV_STRING;
    }
};