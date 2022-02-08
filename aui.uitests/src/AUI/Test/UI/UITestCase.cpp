
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

class MyListener: public ::testing::EmptyTestEventListener {
private:
    static APath saveScreenshot(const AString& testFilePath, const AString& name) noexcept {
        if (!AWindow::current()) return {};
        auto image = AWindow::current()->getRenderingContext()->makeScreenshot();
        if (image.getData().empty()) return {};
        auto p = APath("reports")[APath(testFilePath).filenameWithoutExtension()];
        p.makeDirs();
        p = p[name];
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
                    Render::drawRectBorder(ASolidBrush{0xaae00000_argb},
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
    AWindow::destroyWindowManager();
}
