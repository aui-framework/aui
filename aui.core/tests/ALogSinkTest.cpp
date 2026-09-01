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
#include <AUI/Logging/ALogger.h>
#include <AUI/Logging/ALogSink.h>
#include <AUI/Logging/AConsoleSink.h>
#include <AUI/Logging/AFileSink.h>
#include <AUI/IO/APath.h>
#include <AUI/Common/AVector.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace {

class MockSink : public ALogSink {
public:
    AVector<ALogMessage> messages;
    int flushCount = 0;

    void write(const ALogMessage& message) override {
        messages.push_back(message);
    }

    void flush() override {
        ++flushCount;
    }

    std::string_view name() const noexcept override {
        return "mock";
    }
};

std::string readFile(const APath& path) {
    std::ifstream ifs(path.c_str());
    return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
}

}

TEST(ALogSinkTest, SinkReceivesMessage) {
    ALogger logger;
    auto mock = _new<MockSink>();
    logger.sinks().clear();
    logger.sinks().push_back(mock);

    logger.log(ALogger::INFO, "TestTag") << "hello world";

    ASSERT_EQ(mock->messages.size(), 1u);
    EXPECT_EQ(mock->messages[0].level, static_cast<int>(ALogger::INFO));
    EXPECT_EQ(mock->messages[0].prefix, "TestTag");
    EXPECT_EQ(mock->messages[0].message, "hello world");
}

TEST(ALogSinkTest, MultipleSinksReceiveMessage) {
    ALogger logger;
    auto mock1 = _new<MockSink>();
    auto mock2 = _new<MockSink>();
    logger.sinks().clear();
    logger.sinks().push_back(mock1);
    logger.sinks().push_back(mock2);

    logger.log(ALogger::WARN, "Tag") << "warning";

    EXPECT_EQ(mock1->messages.size(), 1u);
    EXPECT_EQ(mock2->messages.size(), 1u);
    EXPECT_EQ(mock1->messages[0].message, "warning");
    EXPECT_EQ(mock2->messages[0].message, "warning");
}

TEST(ALogSinkTest, EmptyMessageUsesPrefix) {
    ALogger logger;
    auto mock = _new<MockSink>();
    logger.sinks().clear();
    logger.sinks().push_back(mock);

    logger.log(ALogger::INFO, "TagOnly");

    ASSERT_EQ(mock->messages.size(), 1u);
    EXPECT_EQ(mock->messages[0].prefix, "TagOnly");
    EXPECT_EQ(mock->messages[0].message, "TagOnly");
}

TEST(ALogSinkTest, LevelFiltering) {
    ALogger logger;
    auto mock = _new<MockSink>();
    logger.sinks().clear();
    logger.sinks().push_back(mock);
    logger.setLevel(ALogger::ERR);

    logger.log(ALogger::DEBUG, "T") << "dropped";
    logger.log(ALogger::ERR, "T") << "kept";

    ASSERT_EQ(mock->messages.size(), 1u);
    EXPECT_EQ(mock->messages[0].message, "kept");
}

TEST(ALogSinkTest, DisabledLevelDropsAll) {
    ALogger logger;
    auto mock = _new<MockSink>();
    logger.sinks().clear();
    logger.sinks().push_back(mock);
    logger.setLevel(ALogger::DISABLED);

    logger.log(ALogger::ERR, "T") << "dropped";

    EXPECT_EQ(mock->messages.size(), 0u);
}

TEST(ALogSinkTest, MessageFieldsPopulated) {
    ALogger logger;
    auto mock = _new<MockSink>();
    logger.sinks().clear();
    logger.sinks().push_back(mock);

    logger.log(ALogger::TRACE, "MyTag") << "debug msg";

    ASSERT_EQ(mock->messages.size(), 1u);
    auto& msg = mock->messages[0];
    EXPECT_EQ(msg.level, static_cast<int>(ALogger::TRACE));
    EXPECT_EQ(msg.prefix, "MyTag");
    EXPECT_EQ(msg.message, "debug msg");
    EXPECT_EQ(msg.timestamp.size(), 8u);
    EXPECT_EQ(msg.timestamp[2], ':');
    EXPECT_EQ(msg.timestamp[5], ':');
}

TEST(ALogSinkTest, FlushCalledOnDestruction) {
    auto logger = std::make_unique<ALogger>();
    auto mock = _new<MockSink>();
    logger->sinks().clear();
    logger->sinks().push_back(mock);
    logger.reset();
    EXPECT_EQ(mock->flushCount, 1);
}

TEST(ALogSinkTest, OnLoggedCallback) {
    ALogger logger;
    logger.sinks().clear();

    AString capturedPrefix;
    AString capturedMessage;
    ALogger::Level capturedLevel = ALogger::DISABLED;

    logger.onLogged([&](const AString& prefix, const AString& message, ALogger::Level level) {
        capturedPrefix = prefix;
        capturedMessage = message;
        capturedLevel = level;
    });

    logger.log(ALogger::WARN, "CallbackTag") << "cb message";

    EXPECT_EQ(capturedPrefix, "CallbackTag");
    EXPECT_EQ(capturedMessage, "cb message");
    EXPECT_EQ(capturedLevel, ALogger::WARN);
}

TEST(ALogSinkTest, OnLoggedRespectsLevel) {
    ALogger logger;
    logger.sinks().clear();
    logger.setLevel(ALogger::ERR);

    int callCount = 0;
    logger.onLogged([&](const AString&, const AString&, ALogger::Level) { ++callCount; });

    logger.log(ALogger::INFO, "T") << "dropped";
    logger.log(ALogger::ERR, "T") << "kept";

    EXPECT_EQ(callCount, 1);
}

TEST(ALogSinkTest, ConsoleSinkName) {
    AConsoleSink sink;
    EXPECT_EQ(sink.name(), "console");
}

TEST(ALogSinkTest, FileSinkName) {
    auto path = APath::getDefaultPath(APath::TEMP) / "aui_test_filesink.log";
    AFileSink sink(path);
    EXPECT_EQ(sink.name(), "file");
    EXPECT_EQ(sink.path(), path);
}

TEST(ALogSinkTest, FileSinkWritesToFile) {
    auto path = APath::getDefaultPath(APath::TEMP) / "aui_test_filesink_output.log";
    std::remove(path.c_str());

    {
        ALogger logger;
        logger.sinks().clear();
        logger.sinks().push_back(_new<AFileSink>(path));
        logger.log(ALogger::INFO, "TestTag") << "file output";
    }

    auto content = readFile(path);
    EXPECT_NE(content.find("file output"), std::string::npos) << content;
    EXPECT_NE(content.find("TestTag"), std::string::npos) << content;
    EXPECT_NE(content.find("INFO"), std::string::npos) << content;

    std::remove(path.c_str());
}

TEST(ALogSinkTest, DefaultSinks) {
    auto sinks = ALogger::defaultSinks();
    ASSERT_EQ(sinks.size(), 1u);
#if AUI_PLATFORM_ANDROID
    EXPECT_EQ(sinks[0]->name(), "android");
#elif AUI_PLATFORM_APPLE
    EXPECT_EQ(sinks[0]->name(), "apple");
#else
    EXPECT_EQ(sinks[0]->name(), "console");
#endif
}
