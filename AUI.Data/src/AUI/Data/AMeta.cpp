
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
	Autumn::get<ASqlDatabase>()->execute("INSERT OR REPLACE INTO aui_meta (meta_key, meta_value) VALUES (?, ?)", { key, value });
}
