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

void UITestCaseScope::test_unit_aborted(const test_unit& unit) {
    test_observer::test_unit_aborted(unit);

    // draw red rects to highlight views
    for (auto& v : Matcher::current()->toSet()) {
        Render::drawRectBorder(ASolidBrush{ 0xaae00000_argb },
                               v->getPositionInWindow() - glm::ivec2{ 1, 1 },
                               v->getSize() + glm::ivec2{ 2, 2 });
    }

    auto image = AWindow::current()->getRenderingContext()->makeScreenshot();
    APath p("reports");
    p.makeDirs();
    p = p["{}_{}.png"_format(unit.p_name->c_str(), unit.p_line_num)];
    FileOutputStream fos(p);
    PngImageLoader::save(fos, image);

    std::cout << std::string(unit.p_file_name.begin(),  unit.p_file_name.end())
              << '(' << unit.p_line_num << "): report saved at "
              << p.absolute().systemSlashDirection().toStdString()
              << std::endl;
    BOOST_TEST_MESSAGE("azaza");
}
