//
// Created by Alex2772 on 12/18/2021.
//
#include <AUI/Image/PngImageLoader.h>
#include <AUI/IO/FileOutputStream.h>
#include "UITestCase.h"
#include "Matcher.h"
#include <boost/test/tree/test_unit.hpp>
#include <boost/test/unit_test_log.hpp>
#include <AUI/Traits/strings.h>

using namespace boost::unit_test;

APath saveScreenshot(const AString& testFilePath, const AString& name) {
    auto image = AWindow::current()->getRenderingContext()->makeScreenshot();
    if (image.getData().empty()) return {};
    auto p = APath("reports")[APath(testFilePath).filenameWithoutExtension()];
    p.makeDirs();
    p = p[name];
    FileOutputStream fos(p);
    PngImageLoader::save(fos, image);
    return p;
}

void UITestCaseScope::test_unit_aborted(const test_unit& unit) {
    test_observer::test_unit_aborted(unit);

    // draw red rects to highlight views
    if (auto matcher = Matcher::current()) {
        for (auto& v: matcher->toSet()) {
            Render::drawRectBorder(ASolidBrush{0xaae00000_argb},
                                   v->getPositionInWindow() - glm::ivec2{1, 1},
                                   v->getSize() + glm::ivec2{2, 2});
        }
    }

    auto name = std::string(unit.p_file_name.begin(),  unit.p_file_name.end());
    auto p = saveScreenshot(name, "abort-{}_{}.png"_format(unit.p_name->c_str(), unit.p_line_num));

    if (!p.empty()) {
        std::cout << name
                  << '(' << unit.p_line_num << "): report saved at "
                  << p.absolute().systemSlashDirection().toStdString()
                  << std::endl;
    }
}

void UITestCaseScope::test_unit_finish(const test_unit& unit, unsigned long i) {
    UITest::frame();
    test_observer::test_unit_finish(unit, i);

    auto name = std::string(unit.p_file_name.begin(),  unit.p_file_name.end());
    auto p = saveScreenshot(name, "finish-{}_{}.png"_format(unit.p_name->c_str(), unit.p_line_num));

    if (!p.empty()) {
        std::cout << name
                  << '(' << unit.p_line_num << "): screenshot saved at "
                  << p.absolute().systemSlashDirection().toStdString()
                  << std::endl;
    }

    delete this;
}
