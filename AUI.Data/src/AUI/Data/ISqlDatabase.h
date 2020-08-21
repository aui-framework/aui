#pragma once
#include "AUI/Common/SharedPtrTypes.h"
#include "ISqlDriverResult.h"
#include "AUI/Common/AVariant.h"
#include "AUI/Common/AVector.h"
#include "AUI/Data/ASqlDriverType.h"

/*
 * \brief См. ASqlDatabase 
 */
class ISqlDatabase
{
public:
	virtual ~ISqlDatabase() = default;
	virtual _<ISqlDriverResult> query(const AString& query, const AVector<AVariant>& params) = 0;
	virtual int execute(const AString& query, const AVector<AVariant>& params) = 0;

	virtual SqlDriverType getDriverType() = 0;
};
