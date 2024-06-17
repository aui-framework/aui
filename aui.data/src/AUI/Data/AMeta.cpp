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
