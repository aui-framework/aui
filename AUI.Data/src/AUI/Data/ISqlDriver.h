#pragma once
#include <AUI/Common/AString.h>

class ISqlDatabase;

class ISqlDriver
{
public:
	virtual AString getDriverName() = 0;
	virtual _<ISqlDatabase> openDriverConnection(const AString& address, uint16_t port,
		const AString& databaseName, const AString& username, const AString& password) = 0;
};
