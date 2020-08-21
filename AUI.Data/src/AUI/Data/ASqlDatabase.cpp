#include "ASqlDatabase.h"

#include <AUI/Common/AException.h>
#include <AUI/Logging/ALogger.h>
#include "ISqlDatabase.h"
#include "AUI/Common/Plugin.h"


AMap<AString, _<ISqlDriver>>& ASqlDatabase::getDrivers()
{
	static AMap<AString, _<ISqlDriver>> drivers;
	return drivers;
}

ASqlDatabase::~ASqlDatabase() = default;

_<ASqlQueryResult> ASqlDatabase::query(const AString& query, const AVector<AVariant>& params)
{
	return _<ASqlQueryResult>(new ASqlQueryResult(mDriverInterface->query(query, params)));
}

int ASqlDatabase::execute(const AString& query, const AVector<AVariant>& params)
{
	return mDriverInterface->execute(query, params);
}

_<ASqlDatabase> ASqlDatabase::connect(const AString& driverName, const AString& address, uint16_t port,
	const AString& databaseName, const AString& username, const AString& password)
{
	for (int i = 0; i < 2; ++i) {
		if (auto c = getDrivers().contains(driverName))
		{
			return _<ASqlDatabase>(new ASqlDatabase(c->second->openDriverConnection(address, port, databaseName, username, password), driverName));
		}
		else if (i == 0)
		{
			try {
				aui::importPlugin(driverName);
			} catch (const AException& e)
			{
			    ALogger::err(e.getMessage());
				break;
			}
		}
	}
	throw AException("No such driver: " + driverName);
}

void ASqlDatabase::registerDriver(_<ISqlDriver> driver)
{
	getDrivers()[driver->getDriverName()] = driver;
}

SqlDriverType ASqlDatabase::getDriverType() {
    return mDriverInterface->getDriverType();
}
