/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <AUI/Platform/ASingleInstance.h>
#include <AUI/Platform/AApplication.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Util/kAUI.h>
#include <atomic>
#include <thread>

using namespace std::chrono_literals;

TEST(SingleInstance, SerializeRoundtrip) {
    AActivation a {
        .args = { "app", "--flag", "привет", "" },
        .workingDir = "/tmp/dir with spaces",
        .activationToken = "token_123",
    };
    auto b = aui::detail::single_instance::deserialize(aui::detail::single_instance::serialize(a));
    EXPECT_EQ(a.args, b.args);
    EXPECT_EQ(a.workingDir, b.workingDir);
    EXPECT_EQ(a.activationToken, b.activationToken);
}

TEST(SingleInstance, DeserializeGarbage) {
    EXPECT_THROW(aui::detail::single_instance::deserialize("hello"), AException);
    EXPECT_THROW(aui::detail::single_instance::deserialize(""), AException);
}

TEST(SingleInstance, SanitizeKey) {
    EXPECT_EQ(aui::detail::single_instance::sanitizeKey("ru.alex2772.auiwarden"), "ru.alex2772.auiwarden");
    EXPECT_EQ(aui::detail::single_instance::sanitizeKey("a/b\\c d"), "a_b_c_d");
}

#if !AUI_PLATFORM_EMSCRIPTEN
#if AUI_PLATFORM_UNIX
#include <unistd.h>
#include <cstdlib>
static void removeLockFile(const AString& key) {
    auto dir = std::getenv("XDG_RUNTIME_DIR") ? APath(std::getenv("XDG_RUNTIME_DIR")) : APath("/tmp");
    ::unlink((dir / "aui-{}-{}.lock"_format(key, getuid())).toStdString().c_str());
}
#else
static void removeLockFile(const AString&) {}
#endif

// within a single process, the second acquire behaves exactly as a second process would (both flock and named mutex
// are per open file description/handle).
TEST(SingleInstance, SecondAcquireForwardsActivation) {
    const auto key = "aui.test.single_instance.{}"_format(ARandom().nextInt() & 0xffffff);
    AUI_DEFER { removeLockFile(key); };
    std::atomic_int calls = 0;
    AActivation received;
    std::mutex sync;

    auto primary = ASingleInstance::acquire(key, [&](AActivation a) {
        std::unique_lock lock(sync);
        received = std::move(a);
        ++calls;
    });
    ASSERT_NE(primary, nullptr);

    auto secondary = ASingleInstance::acquire(key, {});
    EXPECT_EQ(secondary, nullptr);

    // forwarding is synchronous: the primary acknowledges before the callback is called, so give it a moment
    for (int i = 0; i < 100 && calls == 0; ++i) {
        std::this_thread::sleep_for(10ms);
    }
    ASSERT_EQ(calls, 1);
    std::unique_lock lock(sync);
    EXPECT_EQ(received.workingDir, APath::workingDir());
}

TEST(SingleInstance, LockIsReleasedOnDestruction) {
    const auto key = "aui.test.single_instance.{}"_format(ARandom().nextInt() & 0xffffff);
    AUI_DEFER { removeLockFile(key); };
    {
        auto primary = ASingleInstance::acquire(key, {});
        ASSERT_NE(primary, nullptr);
    }
    auto again = ASingleInstance::acquire(key, {});
    EXPECT_NE(again, nullptr);
}
#endif

TEST(Application, HoldCount) {
    auto& app = AApplication::inst();
    const auto base = app.holdCount();
    {
        auto h1 = app.hold();
        auto h2 = app.hold();
        EXPECT_EQ(app.holdCount(), base + 2);
    }
    EXPECT_EQ(app.holdCount(), base);
}

TEST(Application, DefaultAppIdIsRejected) {
    // aui_app is not called for tests, so app_id is the default one
    EXPECT_THROW(AApplication::inst().requestSingleInstanceLock(), AException);
}
