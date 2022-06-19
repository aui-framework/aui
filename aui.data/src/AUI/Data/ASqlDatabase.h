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

#pragma once

#include <AUI/Data.h>
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/AMap.h"
#include "ISqlDriver.h"
#include "AUI/Common/AVector.h"
#include "AUI/Common/AVariant.h"
#include "ASqlQueryResult.h"
#include "ASqlDriverType.h"

class AString;

class API_AUI_DATA ASqlDatabase
{
private:
	static AMap<AString, _<ISqlDriver>>& getDrivers();

	_<ISqlDatabase> mDriverInterface;

	explicit ASqlDatabase(const _<ISqlDatabase>& driver_interface, const AString& driverName)
		: mDriverInterface(driver_interface)
	{
	}

public:
	~ASqlDatabase();

    /**
     * @brief Execute a query with the result (SELECT).
     *
     * @param query the SQL query
     * @param params query arguments
     * @return query result
     * \throws SQLException when any error occurs
     */
	_<ASqlQueryResult> query(const AString& query, const AVector<AVariant>& params = {});

	/**
	 * @brief Execute a query with no result (UPDATE, INSERT, DELETE, etc.)
     *
     * @param query the SQL query
     * @param params query arguments
     * @return number of affected rows
     * \throws SQLException if any error occurs
	 */
	int execute(const AString& query, const AVector<AVariant>& params = {});


	/**
	 * @brief Connect to the database using the specified details and driver.
     * @param drivername name of the database driver. If the driver is not loaded, an attempt will be made to load the
     *        driver based on the template aui.DRIVERNAME (aui.DRIVERNAME.dll for Windows, libaui.DRIVERNAME.so for
     *        Linux, etc...). If the driver failed to load SQLException will be thrown.
     *
     * @param address server host (IP address or domain)
     * @param port server port
     * @param databaseName name of the database
     * @param username user name; optional in some DBMS
     * @param password user password; optional in some DBMS
     * @return object for communicating with the database; in some DBMS - optional
     * \throws SQLException when any error occurs
	 */
	static _<ASqlDatabase> connect(const AString& driverName, const AString& address, uint16_t port = 0,
	                               const AString& databaseName = {}, const AString& username = {},
	                               const AString& password = {});

	/**
	 * @brief the type of the driver. Required to correct queries in the database due to driver differences.
     * @return type of driver
	 */
	SqlDriverType getDriverType();

	static void registerDriver(_<ISqlDriver> driver);
};
