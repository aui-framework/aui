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

#include <Model/Note.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Url/AUrl.h>
#include <Frame/CoursesFrame.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/ATextField.h>
#include <AUI/Model/AListModel.h>
#include <AUI/Model/AListModelAdapter.h>
#include <AUI/Util/ADataBinding.h>

MainWindow::MainWindow() : AWindow("Notes", 300, 400) {
    setLayout(_new<AHorizontalLayout>());

    Autumn::put(_new<AListModel<Note>>(AListModel<Note>{
        Note{0, "Kek", "lol"},
        Note{1, "Kek", "lol"},
    }));


    addView(_container<AVerticalLayout>({
        _container<AHorizontalLayout>({
            _new<AButton>("Добавить").connect(&AButton::clicked, this, [](){
                Autumn::get<AListModel<Note>>() << Note{7, "Azaza", ""};
            }),
            mDeleteButton = _new<AButton>("Удалить") by(AButton, {
                setDisabled();
            }).connect(&AButton::clicked, this, [&]() {
                //Autumn::get<AListModel<Note>>()->remove(mList->getSelectionModel()->);
            }),
        }),
        mList = _new<AListView>(AAdapter::make(_cast<IListModel<Note>>(Autumn::get<AListModel<Note>>()), [](const Note& n) {
            return n.name + " (" + AString::number(n.id) + ")";
        })) by(AListView, {
            setExpanding({2, 2});
        }).connect(&AListView::selectionChanged, this, [&](const AModelSelection<AVariant>& selection) {
            mDeleteButton->setDisabled(selection.empty());
        })
    }));

    static auto binding = _new<ADataBinding<Note>>();

    addView(_container<AVerticalLayout>({
        _container<AHorizontalLayout>({
            _new<ALabel>("Название") && binding->link(&Note::name, &ALabel::setText),
            _new<ATextField>() && binding->link(&Note::name, &ATextField::textChanging, &ATextField::setText),
        }),
        _new<ALabel>("Текст"),
        _new<ATextField>() by(ATextField, {
            setExpanding({2, 2});
        })
    }));

    Note m;
    m.name = "ASdas";
    binding->setModel(m);
}
