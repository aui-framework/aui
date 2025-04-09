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
#include <AUI/Util/AWordWrappingEngine.h>
#include <AUI/Util/AWordWrappingEngineImpl.h>

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const glm::tvec2<T>& v) {
    return o << "{ " << v.x << ", " << v.y << " }";
}

class MyEntry: public AWordWrappingEngineBase::Entry {
private:
    glm::ivec2 mSize;
    glm::ivec2 mExpectedPosition;

public:
    MyEntry(const glm::ivec2& size, const glm::ivec2& expectedPosition) : mSize(size),
                                                                          mExpectedPosition(expectedPosition) {}

    glm::ivec2 getSize() override {
        return mSize;
    }

    void setPosition(glm::ivec2 position) override {
        EXPECT_NEAR(float(mExpectedPosition.x), float(position.x), 2.f);
        EXPECT_NEAR(float(mExpectedPosition.y), float(position.y), 2.f);
    }

    AFloat getFloat() const override {
        return AFloat::NONE;
    }
};

class FloatingEntry: public MyEntry {
private:
    AFloat mFloating;

public:
    FloatingEntry(const glm::ivec2& size, const glm::ivec2& expectedPosition, AFloat floating) : MyEntry(size,
                                                                                                         expectedPosition),
                                                                                                 mFloating(floating) {}

    AFloat getFloat() const override {
        return mFloating;
    }
};

TEST(WordWrappingEngine, SimpleLeft) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 0}),
        _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{30, 0}),
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{50, 0}),
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 15}),
    });
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, SimpleLeftFloatLeft) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
        _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{0, 0}, AFloat::LEFT),
        _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{30, 0}),
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{50, 0}),
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{30, 15}),
    });
    engine.performLayout({0, 0}, {100, 100});
}
TEST(WordWrappingEngine, SimpleLeftFloatLeftStartFromBeginning) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
        _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{0, 0}, AFloat::LEFT),
        _new<MyEntry>(glm::ivec2{60, 15}, glm::ivec2{30, 0}),
        _new<MyEntry>(glm::ivec2{60, 15}, glm::ivec2{30, 15}),
        _new<MyEntry>(glm::ivec2{60, 0}, glm::ivec2{0, 30}),
    });
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, SimpleLeftFloatRight1) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{70, 0}, AFloat::RIGHT),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{00, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{20, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{00, 15}),
    });
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, SimpleLeftFloatRight2) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{70, 0}, AFloat::RIGHT),
            _new<MyEntry>(glm::ivec2{50, 15}, glm::ivec2{0, 00}),
            _new<MyEntry>(glm::ivec2{50, 20}, glm::ivec2{0, 15}),
            _new<MyEntry>(glm::ivec2{50, 10}, glm::ivec2{0, 35}),
            _new<MyEntry>(glm::ivec2{50, 10}, glm::ivec2{50, 35}),
    });
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, SimpleLeftFloatBoth) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{0, 0}, AFloat::LEFT),
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{70, 0}, AFloat::RIGHT),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{30, 00}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{50, 00}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{30, 15}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{50, 15}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{00, 30}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{20, 30}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{40, 30}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{60, 30}),
    });
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, SimpleRight) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{20, 0}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{50, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{70, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{70, 15}),
    });
    engine.setTextAlign(ATextAlign::RIGHT);
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, SimpleCenter) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{10, 0}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{40, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{60, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{35, 15}),
    });
    engine.setTextAlign(ATextAlign::CENTER);
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, SimpleJustify) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 0}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{40, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{70, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 15}),
    });
    engine.setTextAlign(ATextAlign::JUSTIFY);
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, FloatingEntryConsumesHeight) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngineBase::Entry>>{
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 0}),
        _new<FloatingEntry>(glm::ivec2{10, 100}, glm::ivec2{90, 0}, AFloat::RIGHT),
    });
    engine.performLayout({0, 0}, {100, 100});
    EXPECT_EQ(*engine.height(), 100);
}
