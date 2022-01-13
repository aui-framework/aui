//
// Created by Alex2772 on 12/18/2021.
//
#include <AUI/Image/PngImageLoader.h>
#include <AUI/IO/AFileOutputStream.h>
#include "UITestCase.h"
#include "UIMatcher.h"
#include <AUI/Traits/strings.h>


APath saveScreenshot(const AString& testFilePath, const AString& name) {
    auto image = AWindow::current()->getRenderingContext()->makeScreenshot();
    if (image.getData().empty()) return {};
    auto p = APath("reports")[APath(testFilePath).filenameWithoutExtension()];
    p.makeDirs();
    p = p[name];
    AFileOutputStream fos(p);
    PngImageLoader::save(fos, image);
    return p;
}

void testing::UITest::SetUp() {
    testing::UnitTest::GetInstance()->listeners().Append(this);
    Test::SetUp();
    Render::setRenderer(std::make_unique<SoftwareRenderer>());
    AWindow::setWindowManager<UITestWindowManager>();
    ABaseWindow::currentWindowStorage() = nullptr;
}

void testing::UITest::TearDown() {
    testing::UnitTest::GetInstance()->listeners().Release(this);
    Test::TearDown();
    AWindow::destroyWindowManager();
}

void testing::UITest::OnTestPartResult(const testing::TestPartResult& result) {
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

        auto p = saveScreenshot(result.file_name(), "fail-{}_{}.png"_format(result.file_name(), result.line_number()));

        if (!p.empty()) {
            std::cout << result.file_name()
                      << '(' << result.line_number() << "): report saved at "
                      << p.absolute().systemSlashDirection().toStdString()
                      << std::endl;
        }
    }
}

void testing::UITest::OnTestCaseEnd(const testing::TestCase& aCase) {
    EmptyTestEventListener::OnTestCaseEnd(aCase);
    auto name = std::string(aCase.name());

    auto p = saveScreenshot(name, "finish-{}_{}.png"_format(name));

    if (!p.empty()) {
        std::cout << name
                  << '(' << name << "): screenshot saved at "
                  << p.absolute().systemSlashDirection().toStdString()
                  << std::endl;
    }
}
