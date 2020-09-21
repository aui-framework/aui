//
// Created by alex2 on 21.09.2020.
//

#include <AUI/Util/Cache.h>
#include "AComboBox.h"
#include <AUI/Image/Drawables.h>
#include <glm/gtc/matrix_transform.hpp>


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
}

void AComboBox::render() {
    ALabel::render();
    auto arrow = Drawables::get(":win/svg/combo.svg");
    auto size = arrow->getSizeHint();
    auto s = (getHeight() - size.y) / 2;
    Render::instance().setTransform(
            glm::translate(glm::mat4(1.f), glm::vec3(getWidth() - s - size.x, s, 0.f)));
    arrow->draw(size);
}

int AComboBox::getContentMinimumWidth() {
    return ALabel::getContentMinimumWidth() + 20;
}

