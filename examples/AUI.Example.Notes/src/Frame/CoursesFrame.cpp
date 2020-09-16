//
// Created by alex2 on 21.08.2020.
//

#include <AUI/View/ALabel.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Data/ASqlModel.h>
#include "CoursesFrame.h"
#include <Model/Note.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ASpacer.h>
#include <AUI/View/AButton.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Layout/AStackedLayout.h>
#include <View/NoteView.h>


CoursesFrame::CoursesFrame() {
    AVIEW_CSS;
    setLayout(_new<AVerticalLayout>());
    setExpanding({1, 1});

    addView(_container<AHorizontalLayout>({
        _new<ASpacer>(),
        _new<ALabel>(u8"Изучение")(&AView::addCssName, ".selected"),
        _new<ALabel>(u8"Словарь"),
        _new<ALabel>(u8"Сасистика"),
        _new<ASpacer>(),
    })(&AView::addCssName, ".topbar"));

    auto list = _container<AVerticalLayout>({})
                         (&AView::addCssName, ".list")
                         (&AView::setExpanding, glm::ivec2{1, 1});
    addView(list);

    for (auto& course : Note::all()->get()) {
        list->addView(_new<CourseView>());
    }
}
