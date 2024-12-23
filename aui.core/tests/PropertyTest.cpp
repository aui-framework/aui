// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Util/kAUI.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

template<typename T>
struct AProperty: aui::noncopyable {
    T raw;
    emits<T> changed;

    AProperty() requires requires { aui::default_initializable<T>; } = default;

    template<aui::convertible_to<T> U>
    AProperty(U&& value) noexcept: raw(std::forward<U>(value)) {}

    template<aui::convertible_to<T> U>
    AProperty& operator=(U&& value) noexcept {
        this->raw = std::forward<U>(value);
        emit changed(this->raw);
        return *this;
    }

    template <ASignalInvokable SignalInvokable>
    void operator^(SignalInvokable&& t) {
        t.invokeSignal(nullptr);
    }

    [[nodiscard]]
    bool operator==(const T& rhs) const noexcept {
        return raw == rhs;
    }

    [[nodiscard]]
    bool operator!=(const T& rhs) const noexcept {
        return raw != rhs;
    }

    [[nodiscard]]
    operator const T&() const noexcept {
        return raw;
    }

    [[nodiscard]]
    const T* operator->() const noexcept {
        return &raw;
    }
};


namespace {
struct User {
    AProperty<AString> name;
};


class Receiver: public AObject {
public:
    MOCK_METHOD(void, receive, (const AString& msg));
};
}

TEST(PropertyTest, DesignatedInitializer) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    EXPECT_EQ(u->name, "Hello");
}

TEST(PropertyTest, ValueCanBeChanged) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    u->name = "World";
    EXPECT_EQ(u->name, "World");
}

TEST(PropertyTest, ValueCanBePassed) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    auto stringIdentity = [](const AString& value) {
        return value;
    };
    EXPECT_EQ(stringIdentity(u->name), "Hello");
}

TEST(PropertyTest, ValueOperatorArrow) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    EXPECT_EQ(u->name->length(), 5);
}

TEST(PropertyTest, ChangedSignal) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    auto receiver = _new<Receiver>();
    AObject::connect(u->name.changed, slot(receiver)::receive);

    EXPECT_CALL(*receiver, receive(AString("World")));
    u->name = "World";
}

template<typename M, typename SetterArg, typename Ignored1>
struct Info {
    using Field = std::decay_t<SetterArg>;
    using Model = M;
    Ignored1(Model::*setter)(SetterArg);
};

template<Info info>
struct APropertyExternal {
    using Info = decltype(info);
    using T = Info::Field;
    Info::Model* model;
    T raw{};
    emits<const T&> changed{};

    template<aui::convertible_to<T> U>
    APropertyExternal& operator=(U&& u) {
        std::invoke(info.setter, model, std::forward<U>(u));
        return *this;
    }
};

TEST(PropertyTest, CustomSetter) {
    class CustomSetter: public AObject {
    public:
        MOCK_METHOD(void, setName, (const AString& msg));

        APropertyExternal<{.setter = &CustomSetter::setName}> name = {this};
    } u;

    EXPECT_CALL(u, setName(AString("World")));
    u.name = "World";
}

