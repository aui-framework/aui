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

#pragma once
#include "ASqlDatabase.h"
#include "AUI/Autumn/Autumn.h"

class AString;

class API_AUI_DATA ASqlBlueprintColumn
{
	friend class ASqlBlueprintTable;
private:
	AString mTableName;
	AString mColumnName;
	AString mType;

	bool mIsUnique = false;
	bool mIsNotNull = false;

	ASqlBlueprintColumn(const AString& table_name, const AString& column_name, const AString& type);

public:
	ASqlBlueprintColumn(const ASqlBlueprintColumn&) = delete;
	~ASqlBlueprintColumn();

	ASqlBlueprintColumn& unique();
	ASqlBlueprintColumn& notNull();
};

class API_AUI_DATA ASqlBlueprintTable
{
private:
	AString mTableName;


	ASqlBlueprintTable(_<ASqlDatabase> db, const AString& name);
public:
	ASqlBlueprintTable(const AString& name):
		ASqlBlueprintTable(Autumn::get<ASqlDatabase>(), name)
	{
	}

	ASqlBlueprintColumn integer(const AString& columnName);
	ASqlBlueprintColumn varchar(const AString& columnName, unsigned size = 0xff);
    ASqlBlueprintColumn text(const AString& columnName);
};
