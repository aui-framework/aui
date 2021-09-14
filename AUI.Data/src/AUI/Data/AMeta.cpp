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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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
