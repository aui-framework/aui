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
