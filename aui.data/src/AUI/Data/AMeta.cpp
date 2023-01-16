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


#include "ASqlDatabase.h"
#include "AUI/Autumn/Autumn.h"

#include "AMeta.h"

static void ensureMetaTableExists()
{
    static _weak<ASqlDatabase> db;
    if (Autumn::get<ASqlDatabase>() != db.lock()) {
        auto newDb = Autumn::get<ASqlDatabase>();
        newDb->execute("CREATE TABLE IF NOT EXISTS aui_meta (meta_key varchar(16), meta_value varchar(32), PRIMARY KEY (meta_key))");
        db = newDb;
    }
}

AVariant AMeta::get(const AString& key)
{
	ensureMetaTableExists();
	
	auto result = Autumn::get<ASqlDatabase>()->query("SELECT meta_value FROM aui_meta WHERE meta_key = ?", {key});

	auto i = result->begin();
	
	if (i != result->end())
	{
		return i->getValue(0);
	}
	
	return nullptr;
}

void AMeta::set(const AString& key, const AVariant& value)
{
	ensureMetaTableExists();
	if (Autumn::get<ASqlDatabase>()->getDriverType() == DT_SQLITE)
	    Autumn::get<ASqlDatabase>()->execute(
	            "INSERT OR REPLACE INTO aui_meta (meta_key, meta_value) VALUES (?, ?)", { key, value });
	else
        Autumn::get<ASqlDatabase>()->execute(
                "INSERT INTO aui_meta (meta_key, meta_value) VALUES (?, ?) ON DUPLICATE KEY UPDATE meta_value = ?",
                { key, value, value });
}
