
// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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

// Created by Alex2772 on 12/18/2021.
//
#include <AUI/Image/png/PngImageLoader.h>
#include <AUI/IO/AFileOutputStream.h>
#include "UITestCase.h"
#include "UIMatcher.h"
#include "AUI/Util/kAUI.h"
#include "AUI/UITestState.h"
#include <AUI/Traits/strings.h>
#include <AUI/UITest.h>
#include <gmock/gmock.h>

class MyListener: public ::testing::EmptyTestEventListener {
private:
    static APath saveScreenshot(const AString& testFilePath, const AString& name) noexcept {
        if (!AWindow::current()) return {};
        auto image = AWindow::current()->getRenderingContext()->makeScreenshot();
        if (image.getData().empty()) return {};
        auto p = APath("reports") / APath(testFilePath).filenameWithoutExtension();
        p.makeDirs();
        p = p / name;
        AFileOutputStream fos(p);
        PngImageLoader::save(fos, image);
        return p;
    }

public:
    MyListener() noexcept = default;

    void OnTestPartResult(const testing::TestPartResult& result) override {
        EmptyTestEventListener::OnTestPartResult(result);

        if (result.failed()) {
            // draw red rects to highlight views
            if (auto matcher = ::UIMatcher::current()) {
                for (auto& v: matcher->toSet()) {
                    Render::rectBorder(ASolidBrush{0xaae00000_argb},
                                       v->getPositionInWindow() - glm::ivec2{1, 1},
                                       v->getSize() + glm::ivec2{2, 2});
                }
            }

            // do some hacking here
            // current_test_info causes deadlock on Linux because it's already locked AddTestPartResult for current thread
            struct ScaryHackingShit {
                void* vtable;
                testing::internal::Mutex mutex;
            };
            auto pShit = reinterpret_cast<ScaryHackingShit*>(testing::UnitTest::GetInstance());
            pShit->mutex.Unlock();
            decltype(auto) info = testing::UnitTest::GetInstance()->current_test_info();
            pShit->mutex.Lock();

            auto p = saveScreenshot(info->test_suite_name(), "fail-{}.png"_format(info->name()));

            if (!p.empty()) {
                std::cout << p.absolute().systemSlashDirection().toStdString()
                          << ": "
                          << info->test_suite_name()
                          << "::"
                          << info->name()
                          << " report saved"
                          << std::endl;
            }
        }
    }

    void OnTestEnd(const testing::TestInfo& info) override {
        EmptyTestEventListener::OnTestEnd(info);


        auto p = saveScreenshot(info.test_suite_name(), "finish-{}.png"_format(info.name()));

        if (!p.empty()) {
            std::cout << p.absolute().systemSlashDirection().toStdString()
                      << ": "
                      << info.test_suite_name()
                      << "::"
                      << info.name()
                      << " final screenshot saved"
                      << std::endl;
        }
    }
};


void testing::UITest::SetUp() {
    do_once {
        testing::UnitTest::GetInstance()->listeners().Append(new MyListener);
    }
    UITestState::beginUITest();
    Test::SetUp();
    Render::setRenderer(std::make_unique<SoftwareRenderer>());
    AWindow::setWindowManager<UITestWindowManager>();
    ABaseWindow::currentWindowStorage() = nullptr;
}

void testing::UITest::TearDown() {
    Test::TearDown();

    AWindow::getWindowManager().removeAllWindows();

    // to process all ui messages
    repeat (10) {
        uitest::frame();
    };

    AWindow::destroyWindowManager();
    Render::setRenderer(nullptr);

}
