/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ASqlBlueprint.h"

ASqlBlueprintColumn::ASqlBlueprintColumn(const AString& table_name, const AString& column_name, const AString& type):
	mTableName(table_name),
	mColumnName(column_name),
	mType(type)
{
}

ASqlBlueprintColumn::~ASqlBlueprintColumn()
{
	AString query;
	query.reserve(0x400);
	query += "ALTER TABLE " + mTableName + " ADD COLUMN " + mColumnName + " ";
	query += mType;

	if (mIsUnique)
		query += " unique";
	if (mIsNotNull)
		query += " not null";


	Autumn::get<ASqlDatabase>()->execute(query);
}

ASqlBlueprintColumn& ASqlBlueprintColumn::unique()
{
	mIsUnique = true;
	return *this;
}

ASqlBlueprintColumn& ASqlBlueprintColumn::notNull()
{
	mIsNotNull = true;
	return *this;
}

ASqlBlueprintTable::ASqlBlueprintTable(_<ASqlDatabase> db, const AString& name):
	mTableName(name)
{
	Autumn::put(db);
	switch (db->getDriverType()) {
	case DT_MYSQL:
        Autumn::get<ASqlDatabase>()->execute("CREATE TABLE IF NOT EXISTS " + name + " (id INT AUTO_INCREMENT UNIQUE)");
        break;

	case DT_SQLITE:
        Autumn::get<ASqlDatabase>()->execute("CREATE TABLE IF NOT EXISTS " + name + " (id INTEGER PRIMARY KEY AUTOINCREMENT)");
        break;
    }
}

ASqlBlueprintColumn ASqlBlueprintTable::integer(const AString& columnName)
{
	return {mTableName, columnName, "INTEGER"};
}

ASqlBlueprintColumn ASqlBlueprintTable::varchar(const AString& columnName, unsigned size)
{
	return {mTableName, columnName, "varchar(" + AString::number(size) + ")"};
}
ASqlBlueprintColumn ASqlBlueprintTable::text(const AString& columnName)
{
	return {mTableName, columnName, "TEXT"};
}
