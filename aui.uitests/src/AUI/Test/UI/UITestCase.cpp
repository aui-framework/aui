
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
#include <AUI/Util/AStubWindowManager.h>

class MyListener: public ::testing::EmptyTestEventListener {
public:
    MyListener() noexcept = default;

    static APath saveScreenshot(const AString& testFilePath, const AString& name) noexcept {
        if (!AWindow::current()) return {};
        auto image = AWindow::current()->getRenderingContext()->makeScreenshot();
        if (image.buffer().empty()) return {};
        auto p = APath("reports") / APath(testFilePath).filenameWithoutExtension();
        p.makeDirs();
        p = p / name;
        AFileOutputStream fos(p);
        PngImageLoader::save(fos, image);
        return p;
    }

    void OnTestPartResult(const testing::TestPartResult& result) override {
        EmptyTestEventListener::OnTestPartResult(result);

        if (result.failed()) {
            // draw red rects to highlight views
            if (auto matcher = ::UIMatcher::current()) {
                for (auto& v: matcher->toSet()) {
                    AWindow::current()->getRenderingContext()->renderer().rectangleBorder(ASolidBrush{0xaae00000_argb},
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
    AUI_DO_ONCE {
        testing::UnitTest::GetInstance()->listeners().Append(new MyListener);
    }
    UITestState::beginUITest();
    Test::SetUp();
    AWindow::setWindowManager<AStubWindowManager>();
    AWindowBase::currentWindowStorage() = nullptr;
}

void testing::UITest::TearDown() {
    AWindow::getWindowManager().closeAllWindows();

    // to process all ui messages
    AUI_REPEAT (10) {
        uitest::frame();
    };

    AWindow::destroyWindowManager();

    Test::TearDown();
    UITestState::endUITest();
}
void testing::UITest::saveScreenshot(const AString& name) {
    auto i = testing::UnitTest::GetInstance();
    if (!i) return;
    auto testCase = i->current_test_case();
    if (!testCase) return;
    auto testInfo = i->current_test_info();
    if (!testInfo) return;
    uitest::frame();
    MyListener::saveScreenshot("", "{}.{}_{}.png"_format(testCase->name(), testInfo->name(), name));
}
