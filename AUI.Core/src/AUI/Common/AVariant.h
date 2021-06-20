/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Core.h>

#include "AString.h"
#include "SharedPtr.h"
#include "AUI/Reflect/AClass.h"

namespace AVariantType {
    enum Type {
        AV_UNKNOWN,
        AV_NULL,
        AV_INT,
        AV_UINT,
        AV_FLOAT,
        AV_DOUBLE,
        AV_STRING,
        AV_BOOL,
    };

    template<typename T>
    inline Type getTypeOf() {
        return AV_UNKNOWN;
    }

    template<>
    inline Type getTypeOf<int>() {
        return AV_INT;
    }
    template<>
    inline Type getTypeOf<unsigned>() {
        return AV_UINT;
    }
    template<>
    inline Type getTypeOf<float>() {
        return AV_FLOAT;
    }
    template<>
    inline Type getTypeOf<double>() {
        return AV_DOUBLE;
    }
    template<>
    inline Type getTypeOf<AString>() {
        return AV_STRING;
    }
    template<>
    inline Type getTypeOf<bool>() {
        return AV_BOOL;
    }

};

struct API_AUI_CORE AVariantHelperBase
{
	virtual ~AVariantHelperBase() = default;

	virtual int toInt() = 0;
	virtual unsigned toUInt() = 0;
	virtual float toFloat() = 0;
	virtual double toDouble() = 0;
	virtual bool toBool() = 0;
	virtual AString toString() = 0;
	virtual size_t hash() = 0;
	virtual AVariantType::Type getType() = 0;
};



template <typename T>
struct AVariantHelper : AVariantHelperBase
{
private:
	T mStored;

public:
	AVariantHelper(T stored)
		: mStored(std::move(stored))
	{
	}
	
	~AVariantHelper() override = default;
	int toInt() override
	{
		return mStored;
	}
	unsigned toUInt() override
	{
		return mStored;
	}
	float toFloat() override
	{
		return mStored;
	}
	double toDouble() override
	{
		return mStored;
	}
	bool toBool() override
	{
		return mStored;
	}
	AString toString() override
	{
		return AClass<T>::name();
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


#include "AVariantHelpers.h"

class API_AUI_CORE AVariant
{
private:
	_<AVariantHelperBase> mStored;
	
public:
	template<typename T>
	AVariant(const T& object)
	{
	    if constexpr (!std::is_null_pointer_v<T>) {
            mStored = _new<AVariantHelper<T>>(object);
        }
	}


	AVariant(const char* object) {
        mStored = _new<AVariantHelper<AString>>(object);
	}
	
	AVariant(const _<AVariantHelperBase>& variant_helper_base)
		: mStored(variant_helper_base)
	{
	}

	template<typename T>
	AVariant(const _<AVariantHelper<T>>& variant_helper)
		: mStored(variant_helper)
	{
	}


	AVariant(): AVariant(nullptr)
	{
		
	}

	[[nodiscard]]
	bool isInt() const noexcept {
		return mStored->getType() == AVariantType::AV_INT;
	}

	[[nodiscard]]
	bool isNull() const noexcept {
		return mStored == nullptr;
	}

	[[nodiscard]]
    inline int toInt() const noexcept
	{
		return mStored->toInt();
	}

	[[nodiscard]]
	bool isUInt() const noexcept {
		return mStored->getType() == AVariantType::AV_UINT;
	}
	[[nodiscard]]
	unsigned toUInt() const noexcept
	{
		return mStored->toInt();
	}


	[[nodiscard]]
	bool isFloat() const noexcept {
		return mStored->getType() == AVariantType::AV_FLOAT;
	}
	[[nodiscard]]
    inline float toFloat() const noexcept
	{
		return mStored->toFloat();
	}

	[[nodiscard]]
	bool isDouble() const noexcept {
		return mStored->getType() == AVariantType::AV_DOUBLE;
	}
	[[nodiscard]]
    inline double toDouble() const noexcept
	{
		return mStored->toDouble();
	}


	[[nodiscard]]
	bool isBool() const noexcept {
		return mStored->getType() == AVariantType::AV_BOOL;
	}
	[[nodiscard]]
    inline bool toBool() const noexcept
	{
		return mStored->toBool();
	}

	[[nodiscard]]
	bool isString() const noexcept {
		return mStored->getType() == AVariantType::AV_STRING;
	}
	[[nodiscard]]
    inline AString toString() const noexcept
	{
		return mStored->toString();
	}

	[[nodiscard]]
    inline size_t hash() const noexcept
	{
		return mStored->hash();
	}

    inline AVariantType::Type getType() const {
        return mStored->getType();
    }

    template<typename T>
    T to() const;

    template<typename T>
    operator T() const {
        return to<T>();
    }
    template<typename T>
    bool operator==(const T& other) const;

    bool operator==(const char* other) const;
    template<typename T>
    bool operator!=(const T& other) const {
        return !(*this == other);
    }
};

template<>
inline int AVariant::to<int>() const {
    return toInt();
}
template<>
inline short AVariant::to<short>() const {
    return toInt();
}
template<>
inline unsigned short AVariant::to<unsigned short>() const {
    return toUInt();
}
template<>
inline long AVariant::to<long>() const {
    return toInt();
}
template<>
inline long long AVariant::to<long long>() const {
    return toInt();
}

template<>
inline unsigned AVariant::to<unsigned>() const {
    return toUInt();
}
template<>
inline unsigned long AVariant::to<unsigned long>() const {
    return toUInt();
}
template<>
inline unsigned long long AVariant::to<unsigned long long>() const {
    return toUInt();
}

template<>
inline float AVariant::to<float>() const {
    return toFloat();
}

template<>
inline double AVariant::to<double>() const {
    return toFloat();
}

template<>
inline AString AVariant::to<AString>() const {
    return toString();
}
template<>
inline APath AVariant::to<APath>() const {
    return toString();
}
template<>
inline bool AVariant::to<bool>() const {
    return toBool();
}
inline std::ostream& operator<<(std::ostream& o, const AVariant& v) {
    o << v.toString();
    return o;
}

inline bool AVariant::operator==(const char* other) const {
    return to<AString>() == other;
}

template<typename T>
inline bool AVariant::operator==(const T& other) const {
    return to<T>() == other;
}

template<>
inline bool AVariant::operator==(const AVariant& other) const {
    return toString() == other.toString();
}
