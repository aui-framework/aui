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
#include <AUI/View/ATextBase.h>

namespace {

class TestEntry : public AWordWrappingEngineBase::Entry {
public:
    explicit TestEntry(glm::ivec2 size, bool forcesNextLine = false)
        : mSize(size), mForcesNextLine(forcesNextLine) {}

    glm::ivec2 getSize() override {
        return mSize;
    }

    void setPosition(glm::ivec2 position) override {
        mLastPosition = position;
        ++mSetPositionCalls;
    }

    [[nodiscard]]
    bool forcesNextLine() const override {
        return mForcesNextLine;
    }

    int setPositionCalls() const {
        return mSetPositionCalls;
    }

private:
    glm::ivec2 mSize;
    bool mForcesNextLine = false;
    glm::ivec2 mLastPosition = {};
    int mSetPositionCalls = 0;
};

class TestTextBase : public ATextBase<> {
public:
    void setEntries(Entries entries) {
        mEngine.setEntries(std::move(entries));
    }

    void performStoredLayoutForWidthForTest(int width) {
        performLayoutForWidth(width);
    }

protected:
    void fillStringCanvas(const _<IRenderer::IMultiStringCanvas>& canvas) override {
    }
};

}   // namespace

TEST(ATextBase, IntrinsicWidthIgnoresPaddingAndMaxSize) {
    auto line1a = _new<TestEntry>(glm::ivec2 { 10, 10 });
    auto line1b = _new<TestEntry>(glm::ivec2 { 15, 10 });
    auto lineBreak = _new<TestEntry>(glm::ivec2 { 0, 10 }, true);
    auto line2 = _new<TestEntry>(glm::ivec2 { 40, 10 });

    TestTextBase text;
    text.setPadding({ .left = 7, .right = 9, .top = 3, .bottom = 5 });
    text.setMaxSize({ 20, 100 });
    text.setEntries({ line1a, line1b, lineBreak, line2 });

    EXPECT_EQ(text.onComputeIntrinsicWidth(-1), 40);
}

TEST(ATextBase, IntrinsicHeightMeasurementDoesNotStoreLayout) {
    auto left = _new<TestEntry>(glm::ivec2 { 15, 10 });
    auto right = _new<TestEntry>(glm::ivec2 { 15, 10 });

    TestTextBase text;
    text.setEntries({ left, right });

    EXPECT_GT(text.onComputeIntrinsicHeight(20), 0);
    EXPECT_EQ(left->setPositionCalls(), 0);
    EXPECT_EQ(right->setPositionCalls(), 0);

    text.performStoredLayoutForWidthForTest(20);
    EXPECT_GT(left->setPositionCalls(), 0);
    EXPECT_GT(right->setPositionCalls(), 0);
}
