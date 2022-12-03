// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

//
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
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
            ASSERT_EQ(foundById.id, u.id);
            ASSERT_EQ(foundById.name, u.name);
            ASSERT_EQ(foundById.age, u.age);
        }

        u.name = "Jenny";
        u.save();
        {
            auto foundById = Account::byId(u.id);
            ASSERT_EQ(foundById.name, "Jenny");
        }

        u.remove();
        ASSERT_THROW(Account::byId(u.id), AException);
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
            ASSERT_EQ(u.id, john.id);
            ASSERT_EQ(u.name, john.name);
            ASSERT_EQ(u.age, john.age);
            ASSERT_TRUE((p.message.contains("cars") || p.message.contains("girlz") || p.message.contains("vodka")));
        }

        for (Post& p : jenny.getPosts()->get()) {
            Account u = p.getUser();
            ASSERT_EQ(u.id, jenny.id);
            ASSERT_EQ(u.name, jenny.name);
            ASSERT_EQ(u.age, jenny.age);
            ASSERT_TRUE((p.message.contains("eat") || p.message.contains("sea") || p.message.contains("stupid")));
        }
    }

BOOST_AUTO_TEST_SUITE_END()*/