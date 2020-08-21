//
// Created by alex2 on 21.08.2020.
//

#include <AUI/View/ALabel.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Data/ASqlModel.h>
#include "CoursesFrame.h"
#include <Model/Course.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ASpacer.h>
#include <AUI/View/AButton.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Layout/AStackedLayout.h>

CoursesFrame::CoursesFrame() {
    AVIEW_CSS;
    setLayout(_new<AVerticalLayout>());
    setExpanding({1, 1});

    for (auto& course : Course::all()->get()) {
        addView(_container<AHorizontalLayout>({
            _container<AVerticalLayout>({
                _new<ALabel>(course.name)
                        (&ALabel::setFontSize, 20dp),
                _new<ALabel>(course.description)
                        (&ALabel::setMultiline, true)
                        (&ALabel::setExpanding, glm::ivec2{1, 0}),
            })(&AView::setExpanding, glm::ivec2{1, 0}),
            _container<AStackedLayout>({_new<AButton>(u8"Изучать")})
        }));
    }
}
