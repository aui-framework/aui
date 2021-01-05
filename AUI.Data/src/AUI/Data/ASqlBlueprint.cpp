/**
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
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
