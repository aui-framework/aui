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
