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
