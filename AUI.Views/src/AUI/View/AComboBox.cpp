//
// Created by alex2 on 21.09.2020.
//

#include <AUI/Util/Cache.h>
#include "AComboBox.h"
#include "AListView.h"
#include <AUI/Image/Drawables.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Platform/AWindow.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Animator/ATranslationAnimator.h>
#include <AUI/Animator/ASizeAnimator.h>

class AComboBoxWindow: public AWindow {
public:
    explicit AComboBoxWindow(const _<IListModel<AString>>& model):
        AWindow("COMBOBOX", 854, 500, AWindow::current())
    {
        AVIEW_CSS;
        setWindowStyle(WS_SYS);
        setLayout(_new<AVerticalLayout>());
        addView(mListView = _new<AListView>(model) let (AListView, {
            setExpanding({2, 2});
        }));
    }

    _<AListView> mListView;
};

AComboBox::AComboBox(const _<IListModel<AString>>& model):
    AComboBox()
{
    setModel(model);
}

AComboBox::AComboBox() {
    AVIEW_CSS;
}

void AComboBox::setModel(const _<IListModel<AString>>& model) {
    mModel = model;
    setSelectionId(0);
}

void AComboBox::setSelectionId(int id) {
    mSelectionId = id;
    if (mModel->listSize() > id) {
        setText(mModel->listItemAt(id));
    }
    emit selectionChanged(id);
}

void AComboBox::render() {
    ALabel::render();
    auto arrow = Drawables::get(":win/svg/combo.svg");
    auto size = arrow->getSizeHint();
    auto s = (getHeight() - size.y) / 2;
    Render::inst().setTransform(
            glm::translate(glm::mat4(1.f), glm::vec3(getWidth() - s - size.x, s, 0.f)));
    arrow->draw(size);
}

int AComboBox::getContentMinimumWidth() {
    return ALabel::getContentMinimumWidth() + 20;
}

void AComboBox::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    AView::onMousePressed(pos, button);
    if (mClickConsumer) {
        mClickConsumer = false;
        return;
    }
    if (!mComboWindow) {
        mComboWindow = _new<AComboBoxWindow>(mModel);
        auto w = getWindow();
        auto currentPos = w->unmapPosition(getPositionInWindow() + w->getWindowPosition()) - w->getWindowPosition();
        auto height = mComboWindow->mListView->getContentFullHeight();
        mComboWindow->setGeometry(currentPos.x, currentPos.y + getHeight(),
                                  (glm::max)(getWidth(), mComboWindow->mListView->getMinimumWidth()), height);
        mComboWindow->setAnimator(_new<ASizeAnimator>(
                glm::ivec2{mComboWindow->getWidth(), 0}) let (ASizeAnimator, {
            setDuration(0.1f);
        }));
        connect(w->mousePressed, this, [&] () {
            if (mComboWindow) {
                mClickConsumer = true;
                destroyWindow();
                AThread::current() << [&]() {
                    mClickConsumer = false;
                };
            }
            AObject::disconnect();
        });
        connect(mComboWindow->mListView->selectionChanged, this, [&](const AModelSelection<AString>& s) {
            if (!s.getIndices().empty()) {
                setSelectionId(s.getIndices().begin()->getRow());
            }
            destroyWindow();
            AObject::disconnect();
        });

        mComboWindow->show();
        mPopup = true;
    } else {
        destroyWindow();
    }
}

void AComboBox::getCustomCssAttributes(AMap<AString, AVariant>& map) {
    AButton::getCustomCssAttributes(map);
    if (mPopup)
        map["pressed"] = true;
}

AComboBox::~AComboBox() {
    if (mComboWindow)
        mComboWindow->quit();
}

void AComboBox::destroyWindow() {
    mPopup = false;
    mComboWindow->quit();
    mComboWindow = nullptr;
    emit customCssPropertyChanged();
}


