//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Data/ASqlBuilder.h>
#include "helper.h"


BOOST_AUTO_TEST_SUITE(Builder)

void seedDatabase() {
    table("users").ins("name").row({"Soso"}).rows({{"Kekos"}, {"Lol"}});
}
BOOST_AUTO_TEST_CASE(Raw) {
        setupSimpleDatabase();

        Autumn::get<ASqlDatabase>()->execute("INSERT INTO users (name) VALUES (?)", {"John"});
        auto res = Autumn::get<ASqlDatabase>()->query("SELECT * FROM users");
        for (auto r : res) {
            BOOST_CHECK_EQUAL(r.getValue(1).toString(), "John");
        }
}

BOOST_AUTO_TEST_CASE(BuilderInsert) {
        setupSimpleDatabase();
        seedDatabase();
}
BOOST_AUTO_TEST_CASE(BuilderInsertId) {
        setupSimpleDatabase();
        id_t id = table("users").ins("name").row({"Soso"}).rowId();
        BOOST_CHECK_EQUAL(table("users").sel("name").where(col("id") == id).get().first().first(), "Soso");
}
BOOST_AUTO_TEST_CASE(BuilderSelect) {
        setupSimpleDatabase();
        seedDatabase();
        AVector<AString> names = {
            "Soso",
                    "Kekos",
                    "Lol",
        };
        size_t index = 0;
        for (auto& row : table("users").sel("id", "name").get()) {
            BOOST_CHECK_EQUAL(row[0], unsigned(index + 1));
            BOOST_CHECK_EQUAL(row[1], names[index++]);
        }
}
BOOST_AUTO_TEST_CASE(BuilderSelectWhere1) {
        setupSimpleDatabase();
        seedDatabase();

        auto validate = [](const AVector<AVector<AVariant>>& result) {
            BOOST_ASSERT(result.size() == 1);
            BOOST_ASSERT(result.first().size() == 1);
            return result.first().first();
        };
        BOOST_CHECK_EQUAL(validate(table("users").sel("name").where(col("id") == 1).get()), "Soso");
        BOOST_CHECK_EQUAL(validate(table("users").sel("name").where(col("id") == 2).get()), "Kekos");
        BOOST_CHECK_EQUAL(validate(table("users").sel("name").where(col("id") == 3).get()), "Lol");
        BOOST_TEST(table("users").sel("name").where(col("id") == 4).get().empty());
}
BOOST_AUTO_TEST_CASE(BuilderSelectWhere2) {
        setupSimpleDatabase();
        seedDatabase();
        BOOST_TEST(table("users").sel("name").where(col("id") == 0 && col("id") == 1).get().empty());
        BOOST_TEST(table("users").sel("name").where(col("id") == 1 && col("name") == "Soso").get().size() == 1);
        auto r = table("users").sel("name").where(col("id") == 3 || col("name") == "Soso").get();
        BOOST_CHECK_EQUAL(r[0].first(), "Soso");
        BOOST_CHECK_EQUAL(r[1].first(), "Lol");

        BOOST_TEST(table("users").sel("name").where((col("id") == 1 && col("id") == 2) || col("name") == "Soso").get().size() == 1);
}
BOOST_AUTO_TEST_CASE(BuilderUpdate) {
        setupSimpleDatabase();
        seedDatabase();

        table("users").update({{"name", "pisos"}});

        for (auto i : table("users").sel("name").get()) {
            BOOST_CHECK_EQUAL(i[0], "pisos");
        }
}
BOOST_AUTO_TEST_CASE(BuilderUpdateWhere) {
        setupSimpleDatabase();
        seedDatabase();

        AVector<AString> names = {
            "pisos",
                    "Kekos",
                    "Lol",
        };

        table("users").update({{"name", "pisos"}}).where(col("id") == 1);

        for (auto i : table("users").sel("id", "name").get()) {
            BOOST_CHECK_EQUAL(i[1], names[i[0].toUInt() - 1]);
        }
}

BOOST_AUTO_TEST_CASE(BuilderDelete) {
        setupSimpleDatabase();
        seedDatabase();

        table("users").remove();

        BOOST_TEST(table("users").select().get().empty());
}

BOOST_AUTO_TEST_CASE(BuilderDeleteWhere) {
        setupSimpleDatabase();
        seedDatabase();

        table("users").remove().where(col("id") == 3);

        auto result = table("users").sel("name").get();
        BOOST_ASSERT(result.size() == 2);

        BOOST_CHECK_EQUAL(result[0][0], "Soso");
        BOOST_CHECK_EQUAL(result[1][0], "Kekos");
}

BOOST_AUTO_TEST_SUITE_END()