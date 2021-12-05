/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include <boost/test/unit_test.hpp>
#include <AUI/Util/AWordWrappingEngine.h>

using namespace boost::unit_test;
namespace boost::test_tools::tt_detail {
    template<typename T>
    struct print_log_value<glm::tvec2<T>> {
        void operator()(std::ostream& o, const glm::tvec2<T>& v) {
            o << "{ " << v.x << ", " << v.y << " }";
        }
    };
}

BOOST_AUTO_TEST_SUITE(WordWrappingEngine)

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
            BOOST_CHECK_CLOSE(float(mExpectedPosition.x), float(position.x), 2.f);
            BOOST_CHECK_CLOSE(float(mExpectedPosition.y), float(position.y), 2.f);
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

    BOOST_AUTO_TEST_CASE(SimpleLeft) {
        AWordWrappingEngine engine;
        engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 0}),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{30, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{50, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{0, 15}),
        });
        engine.performLayout({0, 0}, {100, 100});
    }

    BOOST_AUTO_TEST_CASE(SimpleLeftFloatLeft) {
        AWordWrappingEngine engine;
        engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{0, 0}, Float::LEFT),
            _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{30, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{50, 0}),
            _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{30, 15}),
        });
        engine.performLayout({0, 0}, {100, 100});
    }
    BOOST_AUTO_TEST_CASE(SimpleLeftFloatLeftStartFromBeginning) {
        AWordWrappingEngine engine;
        engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
            _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{0, 0}, Float::LEFT),
            _new<MyEntry>(glm::ivec2{60, 15}, glm::ivec2{30, 0}),
            _new<MyEntry>(glm::ivec2{60, 15}, glm::ivec2{30, 15}),
            _new<MyEntry>(glm::ivec2{60, 0}, glm::ivec2{0, 30}),
        });
        engine.performLayout({0, 0}, {100, 100});
    }

    BOOST_AUTO_TEST_CASE(SimpleLeftFloatRight1) {
        AWordWrappingEngine engine;
        engine.setEntries(AVector<_<AWordWrappingEngine::Entry>>{
                _new<FloatingEntry>(glm::ivec2{30, 20}, glm::ivec2{70, 0}, Float::RIGHT),
                _new<MyEntry>(glm::ivec2{20, 15}, glm::ivec2{00, 0}),
                _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{20, 0}),
                _new<MyEntry>(glm::ivec2{30, 10}, glm::ivec2{00, 15}),
        });
        engine.performLayout({0, 0}, {100, 100});
    }
    BOOST_AUTO_TEST_CASE(SimpleLeftFloatRight2) {
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
    BOOST_AUTO_TEST_CASE(SimpleLeftFloatBoth) {
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

    BOOST_AUTO_TEST_CASE(SimpleRight) {
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
    BOOST_AUTO_TEST_CASE(SimpleCenter) {
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
    BOOST_AUTO_TEST_CASE(SimpleJustify) {
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
BOOST_AUTO_TEST_SUITE_END()