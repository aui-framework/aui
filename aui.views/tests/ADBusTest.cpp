/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if AUI_PLATFORM_LINUX

#include <gtest/gtest.h>
#include <AUI/Platform/linux/ADBus.h>

// These tests exercise aui::dbus converters on in-memory messages; no bus connection is required.

namespace {

/**
 * @brief Serializes value into a fresh message and deserializes it back as T.
 */
template <aui::dbus::convertible Written, aui::dbus::convertible Read = Written>
Read roundtrip(const Written& value) {
    auto msg = aui::ptr::manage_unique(dbus_message_new_signal("/test", "test.Iface", "Signal"), dbus_message_unref);
    {
        DBusMessageIter iter;
        dbus_message_iter_init_append(msg.get(), &iter);
        aui::dbus::iter_append(&iter, value);
    }
    DBusMessageIter iter;
    if (!dbus_message_iter_init(msg.get(), &iter)) {
        throw AException("message has no arguments");
    }
    return aui::dbus::iter_get<Read>(&iter);
}

// signature of org.gnome.Shell.Introspect.GetWindows reply
using WindowProperties = AMap<AString, aui::dbus::Variant>;
using Windows = AMap<uint64_t, WindowProperties>;

}   // namespace

TEST(ADBus, VectorRoundtrip) {
    auto result = roundtrip(AVector<int32_t> { 1, 2, 3 });
    EXPECT_EQ(result, (AVector<int32_t> { 1, 2, 3 }));
}

TEST(ADBus, EmptyVector) {
    // used to read DBUS_TYPE_INVALID as the first element
    AVector<int32_t> result;
    EXPECT_NO_THROW(result = roundtrip(AVector<int32_t> {}));
    EXPECT_TRUE(result.empty());
}

TEST(ADBus, MapRoundtrip) {
    auto result = roundtrip(AMap<AString, int32_t> { { "a", 1 }, { "b", 2 } });
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["a"], 1);
    EXPECT_EQ(result["b"], 2);
}

TEST(ADBus, EmptyMap) {
    // used to fail with "assertion failed: dbus_message_iter_get_arg_type(&sub) == ((int) 'e')"
    AMap<AString, int32_t> result;
    EXPECT_NO_THROW(result = roundtrip(AMap<AString, int32_t> {}));
    EXPECT_TRUE(result.empty());
}

TEST(ADBus, GnomeIntrospectGetWindowsNoWindows) {
    // org.gnome.Shell.Introspect.GetWindows replies with an empty a{ta{sv}} right after login, when there are no
    // windows yet.
    Windows result;
    EXPECT_NO_THROW(result = roundtrip(Windows {}));
    EXPECT_TRUE(result.empty());
}

TEST(ADBus, GnomeIntrospectGetWindowsEmptyProperties) {
    Windows windows;
    windows[1] = { { "title", aui::dbus::Variant(std::string("hello")) }, { "has-focus", aui::dbus::Variant(true) } };
    windows[2] = {};

    auto result = roundtrip(windows);
    ASSERT_EQ(result.size(), 2);
    EXPECT_TRUE(result[2].empty());
    ASSERT_TRUE(result[1].contains("title"));
    EXPECT_EQ(std::get<std::string>(static_cast<const aui::dbus::VariantImpl&>(result[1]["title"])), "hello");
    EXPECT_EQ(std::get<bool>(static_cast<const aui::dbus::VariantImpl&>(result[1]["has-focus"])), true);
}

TEST(ADBus, VariantWithEmptyArray) {
    // variant containing an empty array must be read as an empty AVector<Unknown>
    auto msg = aui::ptr::manage_unique(dbus_message_new_signal("/test", "test.Iface", "Signal"), dbus_message_unref);
    {
        DBusMessageIter iter, variant, array;
        dbus_message_iter_init_append(msg.get(), &iter);
        ASSERT_TRUE(dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "as", &variant));
        ASSERT_TRUE(dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, "s", &array));
        ASSERT_TRUE(dbus_message_iter_close_container(&variant, &array));
        ASSERT_TRUE(dbus_message_iter_close_container(&iter, &variant));
    }
    DBusMessageIter iter;
    ASSERT_TRUE(dbus_message_iter_init(msg.get(), &iter));
    auto result = aui::dbus::iter_get<aui::dbus::Variant>(&iter);
    auto* array = std::get_if<AVector<aui::dbus::Unknown>>(&static_cast<const aui::dbus::VariantImpl&>(result));
    ASSERT_NE(array, nullptr);
    EXPECT_TRUE(array->empty());
}

#endif
