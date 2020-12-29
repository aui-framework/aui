//
// Created by alex2 on 31.08.2020.
//

#include <AUI/Autumn/Autumn.h>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Data/AMigrationManager.h>
#include <AUI/Data/ASqlBlueprint.h>

void setupSimpleDatabase() {
    Autumn::put(ASqlDatabase::connect("sqlite", ":memory:"));
    AMigrationManager mm;
    mm.registerMigration("initial", [&]() {
        ASqlBlueprintTable t("users");
        t.varchar("name");
    });
    mm.doMigration();
}
