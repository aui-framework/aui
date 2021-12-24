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

//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Autumn/Autumn.h>
#include <AUI/Data/AMigrationManager.h>
#include <AUI/Data/ASqlBlueprint.h>
#include <AUI/Data/ASqlModel.h>
/*
// ORM
struct Post : ASqlModel<Post> {
    id_t user_id;
    AString message;

    auto getUser();
};

A_META(Post)
{
    A_SQL_TABLE("posts")

    A_FIELDS {
        return {
                A_FIELD(user_id)
                A_FIELD(message)
        };
    };
};

struct Account : ASqlModel<Account> {
    AString name;
    int age;

    auto getPosts() {
        return hasMany<Post>();
    }
};

auto Post::getUser() {
    return belongsTo<Account>(user_id);
}

A_META(Account)
{
    A_SQL_TABLE("users")

    A_FIELDS {
        return {
                A_FIELD(name)
                A_FIELD(age)
        };
    };
};


BOOST_AUTO_TEST_SUITE(ORM)

    BOOST_AUTO_TEST_CASE(Common) {
        Autumn::put(ASqlDatabase::connect("sqlite", ":memory:"));
        AMigrationManager mm;
        mm.registerMigration("initial", [&]() {
            ASqlBlueprintTable t("users");
            t.varchar("name");
            t.integer("age");
        });
        mm.doMigration();

        Account u;
        u.name = "John";
        u.age = 23;
        u.save();
        {
            auto foundById = Account::byId(u.id);
            BOOST_CHECK_EQUAL(foundById.id, u.id);
            BOOST_CHECK_EQUAL(foundById.name, u.name);
            BOOST_CHECK_EQUAL(foundById.age, u.age);
        }

        u.name = "Jenny";
        u.save();
        {
            auto foundById = Account::byId(u.id);
            BOOST_CHECK_EQUAL(foundById.name, "Jenny");
        }

        u.remove();
        BOOST_CHECK_THROW(Account::byId(u.id), AException);
    }

    BOOST_AUTO_TEST_CASE(One2Many) {
        Autumn::put(ASqlDatabase::connect("sqlite", ":memory:"));
        AMigrationManager mm;
        mm.registerMigration("initial", [&]() {
            ASqlBlueprintTable users("users");
            users.varchar("name");
            users.integer("age");

            ASqlBlueprintTable posts("posts");
            posts.integer("user_id");
            posts.text("message");
        });
        mm.doMigration();

        Account john = Account::make("John", 23);
        Account jenny = Account::make("Jenny", 21);

        Post::make(john.id, "I love cars");
        Post::make(john.id, "im a typical boi girlz add me friends");
        Post::make(john.id, "vodka balalayka");

        Post::make(jenny.id, "I like to eat");
        Post::make(jenny.id, "it's me and my girlfriend on the sea");
        Post::make(jenny.id, "today i gonna eat this sweet cake");
        Post::make(jenny.id, "Boys are so stupid");

        for (Post& p : john.getPosts()->get()) {
            Account u = p.getUser();
            BOOST_CHECK_EQUAL(u.id, john.id);
            BOOST_CHECK_EQUAL(u.name, john.name);
            BOOST_CHECK_EQUAL(u.age, john.age);
            BOOST_TEST((p.message.contains("cars") || p.message.contains("girlz") || p.message.contains("vodka")));
        }

        for (Post& p : jenny.getPosts()->get()) {
            Account u = p.getUser();
            BOOST_CHECK_EQUAL(u.id, jenny.id);
            BOOST_CHECK_EQUAL(u.name, jenny.name);
            BOOST_CHECK_EQUAL(u.age, jenny.age);
            BOOST_TEST((p.message.contains("eat") || p.message.contains("sea") || p.message.contains("stupid")));
        }
    }

BOOST_AUTO_TEST_SUITE_END()*/