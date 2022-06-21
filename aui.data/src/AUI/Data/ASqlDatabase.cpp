/*
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
