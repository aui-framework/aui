#define BOOST_TEST_MODULE Tests
#include <boost/test/included/unit_test.hpp>
#include <AUI/Autumn/Autumn.h>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Data/AMigrationManager.h>
#include <AUI/Data/ASqlBlueprint.h>
#include <AUI/Data/AMeta.h>
#include <AUI/Data/ASqlBuilder.h>
#include <AUI/Data/AModelMeta.h>
#include <AUI/Data/ASqlModel.h>
#include <AUI/Common/ASignal.h>
#include "AUI/Common/AString.h"
#include "AUI/Util/ARandom.h"
#include "AUI/Crypt/ARsa.h"
#include "AUI/Crypt/AX509.h"
#include "AUI/IO/FileInputStream.h"
#include "AUI/Crypt/AX509Store.h"
#include "AUI/Thread/AFuture.h"
#include "AUI/Thread/AThreadPool.h"
#include "AUI/Reflect/AClass.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Util/Util.h"

using namespace boost::unit_test;

// ORM
struct Post: ASqlModel<Post> {
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

struct User: ASqlModel<User> {
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


BOOST_AUTO_TEST_SUITE(Core_AString)

BOOST_AUTO_TEST_CASE(UTF8)
{
	const char* someUtfString = u8"Ютф строка! \u0228";

	AString s = someUtfString;

	BOOST_TEST(s == someUtfString);
	BOOST_TEST(memcmp(s.toUtf8()->data(), someUtfString, strlen(someUtfString)) == 0);

}
BOOST_AUTO_TEST_SUITE_END()



template<typename T>
std::ostream& operator<<(std::ostream& o, const std::atomic<T>& n) {
    o << *n;
    return o;
}

BOOST_AUTO_TEST_SUITE(Core_Threading)

BOOST_AUTO_TEST_CASE(Async)
{
	auto someInt = _new<std::atomic_int>(0);
	for (int i = 0; i < 100; ++i)
		async {
			(*someInt) += 1;
		};

	AThread::sleep(1000);

	BOOST_CHECK_EQUAL(*someInt, 100);
}
BOOST_AUTO_TEST_CASE(Future1, *boost::unit_test::tolerance(10))
{
	ADeque<_<AFuture<double>>> taskList;
	auto time = Util::measureTimeInMillis([&]()
	{
		repeat(1000)
		{
			taskList << async {
				double i = 2.0;
				repeat(1000)
				{
					i *= 2;
				}
				return i;
			};
		}

		taskList.forEach(&AFuture<double>::get);
		
		printf("Ok, result: %f\n", **taskList.first());
	});
	
	printf("Finished in %llu ms\n", time);
	BOOST_CHECK_EQUAL(**taskList.first(), 21430172143725346418968500981200036211228096234110672148875007767407021022498722449863967576313917162551893458351062936503742905713846280871969155149397149607869135549648461970842149210124742283755908364306092949967163882534797535118331087892154125829142392955373084335320859663305248773674411336138752.000000);
	BOOST_TEST(time < 1000);
}
BOOST_AUTO_TEST_CASE(Future2)
{
	auto b = _new<bool>(false);
	async{
		AThread::sleep(1000);
		*b = true;
	};
	BOOST_TEST(!*b);
	AThread::sleep(2000);
	BOOST_TEST(*b);
}
BOOST_AUTO_TEST_CASE(Future3)
{
	auto b = _new<bool>(false);
	auto v8 = async{
		AThread::sleep(1000);
		*b = true;
		return 8;
	};
	auto v1231 = async{
		AThread::sleep(1000);
		*b = true;
		return 1231.f;
	};
	BOOST_TEST(!*b);
	AThread::sleep(2000);
	BOOST_TEST(*b);
	
	BOOST_CHECK_EQUAL(**v8, 8);
	BOOST_CHECK_EQUAL(**v1231, 1231.f);
}
BOOST_AUTO_TEST_CASE(Repeat)
{
	auto someInt = _new<std::atomic_int>(0);
	
	repeat(100'000){
		(*someInt) += 1;
	};

	BOOST_CHECK_EQUAL(*someInt, 100'000);
}
BOOST_AUTO_TEST_CASE(RepeatAsync)
{
	auto someInt = _new<std::atomic_int>(0);
	
	repeat_async(100'000){
		(*someInt) += 1;
	};

	AThread::sleep(1'000);

	BOOST_CHECK_EQUAL(*someInt, 100'000);
}
BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(Crypt)

BOOST_AUTO_TEST_CASE(EncryptDecrypt)
{
	unsigned bits = 512;
	for (uint32_t i = 0; i < 4; ++i, bits <<= 1) {
		auto r = ARsa::generate();
		for (auto size : { 10, 1000, 5000 }) {
			auto someData = ARandom().nextBytes(size);

			auto encrypted = r->encrypt(someData);
			auto decrypted = r->decrypt(encrypted);
			BOOST_CHECK_EQUAL(*decrypted, *someData);
		}
	}

}

BOOST_AUTO_TEST_CASE(PEM_PrivateWrite)
{
	auto r = ARsa::generate();
	AString s = AString::fromLatin1(r->getPrivateKeyPEM());
	BOOST_TEST(s.startsWith("-----BEGIN RSA PRIVATE KEY-----"));
	BOOST_TEST(s.endsWith("-----END RSA PRIVATE KEY-----\n"));
}

BOOST_AUTO_TEST_CASE(PEM_PublicWrite)
{
	auto r = ARsa::generate();
	AString s = AString::fromLatin1(r->getPublicKeyPEM());

	BOOST_TEST(s.startsWith("-----BEGIN RSA PUBLIC KEY-----"));
	BOOST_TEST(s.endsWith("-----END RSA PUBLIC KEY-----\n"));
}

BOOST_AUTO_TEST_CASE(PEM_PrivateRead)
{
	auto both = ARsa::generate();
	auto priv = ARsa::fromPrivateKeyPEM(both->getPrivateKeyPEM());

	auto someData = ARandom().nextBytes(10000);
	auto result = priv->decrypt(both->encrypt(someData));

	BOOST_CHECK_EQUAL(*someData, *result);

}

BOOST_AUTO_TEST_CASE(PEM_PublicRead)
{
	auto both = ARsa::generate();
	auto pub = ARsa::fromPublicKeyPEM(both->getPublicKeyPEM());

	auto someData = ARandom().nextBytes(10000);
	auto result = both->decrypt(pub->encrypt(someData));

	BOOST_CHECK_EQUAL(*someData, *result);

}
/*
BOOST_AUTO_TEST_CASE(X509_read)
{
	auto root = AX509::fromPEM(ByteBuffer::fromStream(_new<FileInputStream>("root.crt")));
	auto nonvalid = AX509::fromPEM(ByteBuffer::fromStream(_new<FileInputStream>("nonvalid.crt")));

	AX509Store s;
	s.addCert(root);
	
	BOOST_TEST(root->isCA());
	BOOST_TEST(!root->checkTrust());

	BOOST_TEST(!root->checkTrust());

	BOOST_TEST(!s.validate(nonvalid));
	BOOST_TEST(!root->checkTrust());
	BOOST_TEST(nonvalid->checkHost("localhost"));
	BOOST_TEST(!nonvalid->checkHost("slocalhost"));
}*/

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(Common)

BOOST_AUTO_TEST_CASE(SharedPtrs)
{
	class SomeBuilderClass
	{
	private:
		AString mUsername;
		int mAge = -1;
	public:
		SomeBuilderClass() {}

		void setUsername(const AString& username)
		{
			mUsername = username;
		}

		void setAge(int age)
		{
			mAge = age;
		}


		const AString& getUsername() const
		{
			return mUsername;
		}

		int getAge() const
		{
			return mAge;
		}
	};
	
	auto builder = _new<SomeBuilderClass>();
	builder(&SomeBuilderClass::setUsername, "John")
		   (&SomeBuilderClass::setAge, 23);

	BOOST_CHECK_EQUAL(builder->getUsername(), "John");
	BOOST_CHECK_EQUAL(builder->getAge(), 23);
}


BOOST_AUTO_TEST_CASE(NullSafety)
{
	class Person
	{
	private:
		AString mName;
		int mAge;
	public:

		Person(const AString& name, int age)
			: mName(name),
			  mAge(age)
		{
		}


		[[nodiscard]] const AString& getName() const
		{
			return mName;
		}

		void setName(const AString& name)
		{
			mName = name;
		}

		[[nodiscard]] int getAge() const
		{
			return mAge;
		}

		void setAge(const int age)
		{
			mAge = age;
		}
	};

	AVector<_<Person>> persons = {
		_new<Person>("John", 23),
		_new<Person>("Jenny", 21),
		nullptr
	};

	for (auto& person : persons)
	{
        person.safe()
			(&Person::setAge, 80)
			(&Person::setName, "Loh")
		;
	}
	BOOST_CHECK_EQUAL(persons[0]->getAge(), 80);
	BOOST_CHECK_EQUAL(persons[1]->getAge(), 80);
	BOOST_CHECK_EQUAL(persons[0]->getName(), "Loh");
	BOOST_CHECK_EQUAL(persons[1]->getName(), "Loh");
	BOOST_CHECK_EQUAL(persons[2], nullptr);
}
class SendObject: public AObject {
public:
    void invokeSignal() {
        emit someSignal();
    }

signals:
    emits<> someSignal;
};
class ReceiverObject: public AObject {
public:
    void receiveSignal() {
        mSignalInvoked = true;
    }

    bool mSignalInvoked = false;
};

BOOST_AUTO_TEST_CASE(ConnectBuilder)
{
    auto receiver = _new<ReceiverObject>();

    auto sender = _new<SendObject>()
            .connect(&SendObject::someSignal, receiver, &ReceiverObject::receiveSignal);


    with(sender, {
       invokeSignal();
    });

    BOOST_TEST(receiver->mSignalInvoked);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(DependencyInjection)

BOOST_AUTO_TEST_CASE(Anonymous) {
    class MyData {
    private:
        AString mString;

    public:
        MyData(const AString& string) : mString(string) {}

        const AString& getString() const {
            return mString;
        }
    };
    Autumn::put(_new<MyData>("hello"));

    BOOST_CHECK_EQUAL(Autumn::get<MyData>()->getString(), "hello");
}
BOOST_AUTO_TEST_CASE(Named) {
    class MyData {
    private:
        AString mString;

    public:
        MyData(const AString& string) : mString(string) {}

        const AString& getString() const {
            return mString;
        }
    };
    Autumn::put("obj_foo", _new<MyData>("foo"));
    Autumn::put("obj_bar", _new<MyData>("bar"));

    BOOST_CHECK_EQUAL(Autumn::get<MyData>("obj_foo")->getString(), "foo");
    BOOST_CHECK_EQUAL(Autumn::get<MyData>("obj_bar")->getString(), "bar");
    BOOST_CHECK_THROW(Autumn::get<MyData>("obj_nonexistent"), AException);
}

BOOST_AUTO_TEST_CASE(Constructor) {
    class BasicComponent {
    private:
        AString mName;

    public:
        BasicComponent(const AString& name) : mName(name) {}
        virtual ~BasicComponent() = default;

        const AString& getName() const {
            return mName;
        }

    };

    class Motherboard: public BasicComponent {
    public:
        Motherboard(const AString& name) : BasicComponent(name) {}
        virtual ~Motherboard() = default;

    };
    class CPU: public BasicComponent {
    public:
        CPU(const AString& name) : BasicComponent(name) {}
        virtual ~CPU() = default;
    };
    class Videocard: public BasicComponent {
    public:
        Videocard(const AString& name) : BasicComponent(name) {}
        virtual ~Videocard() = default;

    };

    class Computer {
    private:
        _<Motherboard> mMotherboard;
        _<CPU> mCPU;
        _<Videocard> mVideocard;

    public:
        Computer(const _<Motherboard>& motherboard, const _<CPU>& cpu, const _<Videocard>& videocard) : mMotherboard(
                motherboard), mCPU(cpu), mVideocard(videocard) {}

        const _<Motherboard>& getMotherboard() const {
            return mMotherboard;
        }

        const _<CPU>& getCpu() const {
            return mCPU;
        }

        const _<Videocard>& getVideocard() const {
            return mVideocard;
        }
    };

    Autumn::put(_new<Motherboard>("B450"));
    Autumn::put(_new<CPU>("3600U"));
    Autumn::put(_new<Videocard>("750Ti"));

    auto computer = Autumn::construct<Computer>::with<Motherboard, CPU, Videocard>();

    BOOST_CHECK_EQUAL(computer->getMotherboard()->getName(), "B450");
    BOOST_CHECK_EQUAL(computer->getCpu()->getName(), "3600U");
    BOOST_CHECK_EQUAL(computer->getVideocard()->getName(), "750Ti");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Data)

void setupSimpleDatabase() {
    Autumn::put(ASqlDatabase::connect("sqlite", ":memory:"));
    AMigrationManager mm;
    mm.registerMigration("initial", [&]() {
        ASqlBlueprintTable t("users");
        t.varchar("name");
    });
    mm.doMigration();
}
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


BOOST_AUTO_TEST_CASE(ORM) {
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

BOOST_AUTO_TEST_CASE(ORM_One2Many) {
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

BOOST_AUTO_TEST_CASE(Meta) {
    Autumn::put(ASqlDatabase::connect("sqlite", ":memory:"));
    AMeta::set("kek", 4);
    AMeta::set("lol", "azaza");
    BOOST_CHECK_EQUAL(AMeta::get("kek"), 4);
    BOOST_CHECK_EQUAL(AMeta::get("lol"), "azaza");
    AMeta::set("kek", "four");
    AMeta::set("lol", 42.0);
    BOOST_CHECK_EQUAL(AMeta::get("kek"), "four");
    BOOST_CHECK_EQUAL(AMeta::get("lol"), 42.0);
}


BOOST_AUTO_TEST_SUITE_END()