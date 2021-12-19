//
// Created by Alex2772 on 12/18/2021.
//
#include <AUI/Image/PngImageLoader.h>
#include <AUI/IO/FileOutputStream.h>
#include "UITestCase.h"
#include <boost/test/tree/test_unit.hpp>
#include <boost/test/unit_test_log.hpp>
#include <AUI/Traits/strings.h>

using namespace boost::unit_test;

void UITestCaseScope::test_unit_aborted(const test_unit& unit) {
    test_observer::test_unit_aborted(unit);

    auto image = AWindow::current()->getRenderingContext()->makeScreenshot();
    APath p("reports");
    p.makeDirs();
    p = p["{}_{}.png"_format(unit.p_name->c_str(), unit.p_line_num)];
    FileOutputStream fos(p);
    PngImageLoader::save(fos, image);

    //std::cout << log::begin( unit.p_file_name, unit.p_line_num ) << log_level::log_messages;
    //std::cout << "report saved at " << p.absolute().toStdString();
    //std::cout << log::end();
}
