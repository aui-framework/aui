#pragma once

#include <AUI/Data/ISqlDriver.h>

class AMysql: public ISqlDriver {
public:
	AString getDriverName() override;
	_<ISqlDatabase> openDriverConnection(const AString& address, uint16_t port, const AString& databaseName,
		const AString& username, const AString& password) override;
};