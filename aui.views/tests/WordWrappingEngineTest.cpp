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

#include <gtest/gtest.h>
#include <AUI/Util/AWordWrappingEngine.h>

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const glm::tvec2<T>& v) {
    return o << "{ " << v.x << ", " << v.y << " }";
}

class MyEntry: public AWordWrappingEngine::Entry {
private:
    glm::ivec2 mSize;
    glm::ivec2 mExpectedPosition;

public:
    MyEntry(const glm::ivec2& size, const glm::ivec2& expectedPosition) : mSize(size),
                                                                          mExpectedPosition(expectedPosition) {}

    glm::ivec2 getSize() override {
        return mSize;
    }

    void setPosition(const glm::ivec2& position) override {
        EXPECT_NEAR(float(mExpectedPosition.x), float(position.x), 2.f);
        EXPECT_NEAR(float(mExpectedPosition.y), float(position.y), 2.f);
    }

    Float getFloat() const override {
        return Float::NONE;
    }
};

class FloatingEntry: public MyEntry {
private:
    Float mFloating;

public:
    FloatingEntry(const glm::ivec2& size, const glm::ivec2& expectedPosition, Float floating) : MyEntry(size,
                                                                                                        expectedPosition),
                                                                                                mFloating(floating) {}

    Float getFloat() const override {
        return mFloating;
    }
};

TEST(WordWrappingEngine, SimpleLeft) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 0}),
        _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{30, 0}),
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{50, 0}),
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 15}),
    });
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, SimpleLeftFloatLeft) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
        _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{0, 0}, Float::LEFT),
        _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{30, 0}),
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{50, 0}),
        _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{30, 15}),
    });
    engine.performLayout({0, 0}, {100, 100});
}
TEST(WordWrappingEngine, SimpleLeftFloatLeftStartFromBeginning) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
        _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{0, 0}, Float::LEFT),
        _new<MyEntry>(glm::ivec2{60, 15}, glm::ivec2{30, 0}),
        _new<MyEntry>(glm::ivec2{60, 15}, glm::ivec2{30, 15}),
        _new<MyEntry>(glm::ivec2{60, 0}, glm::ivec2{0, 30}),
    });
    engine.performLayout({0, 0}, {100, 100});
}

TEST(WordWrappingEngine, SimpleLeftFloatRight1) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{70, 0}, Float::RIGHT),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{00, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{20, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{00, 15}),
    });
    engine.performLayout({0, 0}, {100, 100});
}
TEST(WordWrappingEngine, SimpleLeftFloatRight2) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{70, 0}, Float::RIGHT),
            _new<MyEntry>(glm::ivec2{50, 15}, glm::ivec2{0, 00}),
            _new<MyEntry>(glm::ivec2{50, 20}, glm::ivec2{0, 15}),
            _new<MyEntry>(glm::ivec2{50, 10}, glm::ivec2{0, 35}),
            _new<MyEntry>(glm::ivec2{50, 10}, glm::ivec2{50, 35}),
    });
    engine.performLayout({0, 0}, {100, 100});
}
TEST(WordWrappingEngine, SimpleLeftFloatBoth) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{0, 0}, Float::LEFT),
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{70, 0}, Float::RIGHT),
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
    engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{20, 0}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{50, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{70, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{70, 15}),
    });
    engine.setTextAlign(TextAlign::RIGHT);
    engine.performLayout({0, 0}, {100, 100});
}
TEST(WordWrappingEngine, SimpleCenter) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{10, 0}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{40, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{60, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{35, 15}),
    });
    engine.setTextAlign(TextAlign::CENTER);
    engine.performLayout({0, 0}, {100, 100});
}
TEST(WordWrappingEngine, SimpleJustify) {
    AWordWrappingEngine engine;
    engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 0}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{40, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{70, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 15}),
    });
    engine.setTextAlign(TextAlign::JUSTIFY);
    engine.performLayout({0, 0}, {100, 100});
}