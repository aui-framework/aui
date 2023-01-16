// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
	return aui::ptr::manage(new ASqlQueryResult(mDriverInterface->query(query, params)));
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
			return aui::ptr::manage(new ASqlDatabase(c->second->openDriverConnection(address, port, databaseName, username, password), driverName));
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
	getDrivers()[driver->getDriverName()] = std::move(driver);
}

SqlDriverType ASqlDatabase::getDriverType() {
    return mDriverInterface->getDriverType();
}
