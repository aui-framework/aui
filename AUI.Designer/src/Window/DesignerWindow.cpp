//
// Created by alex2 on 05.12.2020.
//

#include "DesignerWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Platform/ACustomCaptionWindow.h>
#include <AUI/Designer/DesignerRegistrator.h>
#include <View/FakeWindow.h>
#include <AUI/View/AScrollArea.h>

DesignerWindow::DesignerWindow():
    AWindow("AUI Designer")
{
    setLayout(_new<AStackedLayout>());

    auto componentsList = _new<AScrollArea>();

    componentsList->getContentContainer()->setLayout(_new<AVerticalLayout>());

    repeat(10) {
        for (auto& c : aui::detail::DesignerRegistrationBase::getRegistrations()) {
            componentsList->getContentContainer()->addView(c->instanciate());
        }
    }

    addView(_container<AVerticalLayout>({
        _container<AHorizontalLayout>({
            _new<AButton>("ты лох")
        }) let (AView, {
            setCss("background: $OS_THEME_COLOR");
        }),
        _container<AHorizontalLayout>({
            // левая колонка с компонентами
            _container<AVerticalLayout>({
                _new<ALabel>("Компоненты") << ".components_title",
                componentsList
            }) let (AViewContainer, {
                setExpanding({0, 2});
            }) << ".components",

            // центральная рабочая область
            _container<AStackedLayout>({
                _new<FakeWindow>()
            }) let (AViewContainer, {
                setExpanding({2, 2});
            })
        }) let (AViewContainer, {
            setExpanding({2, 2});
        })
    }) let (AViewContainer, {
        setExpanding({2, 2});
    }));
    setCss("padding: 0");
}
