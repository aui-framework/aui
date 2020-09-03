//
// Created by alex2772 on 2020-08-11.
//

#include <AUI/View/ALabel.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/View/ASpacer.h>
#include "MainWindow.h"
#include <AUI/Render/AFontManager.h>
#include <AUI/Animator/ARotationAnimator.h>
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Data/AMigrationManager.h>
#include <AUI/Json/AJson.h>
#include <AUI/IO/FileInputStream.h>
#include <AUI/Data/ASqlBlueprint.h>
#include <AUI/Util/UIBuildingHelpers.h>

#include <Model/Course.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Url/AUrl.h>
#include <Frame/CoursesFrame.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/ATextField.h>

MainWindow::MainWindow() : AWindow("Notes", 300, 400) {
    setLayout(_new<AHorizontalLayout>());

    auto x = _new<AStringVector>();
    x << "Kek" << "Lol";

    addView(_container<AVerticalLayout>({
        _container<AHorizontalLayout>({
            _new<AButton>("Добавить"),
            _new<AButton>("Удалить"),
        }),
        _new<AListView>(x) by(AListView, {
            setExpanding({2, 2});
        })
    }));

    addView(_container<AVerticalLayout>({
        _container<AHorizontalLayout>({
            _new<ALabel>("Название"),
            _new<ATextField>(),
        }),
        _new<ALabel>("Текст"),
        _new<ATextField>() by(ATextField, {
            setExpanding({2, 2});
        })
    }));
}
