//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Autumn/Autumn.h>
#include <AUI/Data/AMigrationManager.h>
#include <AUI/Data/ASqlBlueprint.h>
#include <AUI/Data/ASqlModel.h>

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

struct User : ASqlModel<User> {
    AString name;
    int age;

    auto getPosts() {
        return hasMany<Post>();
    }
};

auto Post::getUser() {
    return belongsTo<User>(user_id);
}

A_META(User)
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

        User u;
        u.name = "John";
        u.age = 23;
        u.save();
        {
            auto foundById = User::byId(u.id);
            BOOST_CHECK_EQUAL(foundById.id, u.id);
            BOOST_CHECK_EQUAL(foundById.name, u.name);
            BOOST_CHECK_EQUAL(foundById.age, u.age);
        }

        u.name = "Jenny";
        u.save();
        {
            auto foundById = User::byId(u.id);
            BOOST_CHECK_EQUAL(foundById.name, "Jenny");
        }

        u.remove();
        BOOST_CHECK_THROW(User::byId(u.id), AException);
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

        User john = User::make("John", 23);
        User jenny = User::make("Jenny", 21);

        Post::make(john.id, "I love cars");
        Post::make(john.id, "im a typical boi girlz add me friends");
        Post::make(john.id, "vodka balalayka");

        Post::make(jenny.id, "I like to eat");
        Post::make(jenny.id, "it's me and my girlfriend on the sea");
        Post::make(jenny.id, "today i gonna eat this sweet cake");
        Post::make(jenny.id, "Boys are so stupid");

        for (Post& p : john.getPosts()->get()) {
            User u = p.getUser();
            BOOST_CHECK_EQUAL(u.id, john.id);
            BOOST_CHECK_EQUAL(u.name, john.name);
            BOOST_CHECK_EQUAL(u.age, john.age);
            BOOST_TEST((p.message.contains("cars") || p.message.contains("girlz") || p.message.contains("vodka")));
        }

        for (Post& p : jenny.getPosts()->get()) {
            User u = p.getUser();
            BOOST_CHECK_EQUAL(u.id, jenny.id);
            BOOST_CHECK_EQUAL(u.name, jenny.name);
            BOOST_CHECK_EQUAL(u.age, jenny.age);
            BOOST_TEST((p.message.contains("eat") || p.message.contains("sea") || p.message.contains("stupid")));
        }
    }

BOOST_AUTO_TEST_SUITE_END()