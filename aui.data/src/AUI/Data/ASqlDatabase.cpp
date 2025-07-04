/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
	return aui::ptr::manage_shared(new ASqlQueryResult(mDriverInterface->query(query, params)));
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
			return aui::ptr::manage_shared(new ASqlDatabase(c->second->openDriverConnection(address, port, databaseName, username, password), driverName));
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
