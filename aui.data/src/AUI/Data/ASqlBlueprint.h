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
