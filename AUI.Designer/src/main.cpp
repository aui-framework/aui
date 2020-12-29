#include <AUI/Platform/Entry.h>
#include <Window/DesignerWindow.h>
#include <AUI/Designer/DesignerRegistrator.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/AComboBox.h>
#include <AUI/Model/AListModel.h>
#include <AUI/View/AScrollbar.h>


DESIGNER_REGISTRATION(ALabel, {
    virtual _<AView> instanciate() {
        return _new<ALabel>("Метка");
    }
})

DESIGNER_REGISTRATION(AButton, {
    virtual _<AView> instanciate() {
        return _new<AButton>("Кноп Очка");
    }
})
DESIGNER_REGISTRATION(ATextField, {
    virtual _<AView> instanciate() {
        auto t = _new<ATextField>();
        t->setText("Текстовое поле");
        return t;
    }
})
DESIGNER_REGISTRATION(AComboBox, {
    virtual _<AView> instanciate() {
        return _new<AComboBox>(_new<AListModel<AString>>(AVector<AString>{"Кккккккомбобокс", "Ты чё сюда смотришь?"}));
    }
})
DESIGNER_REGISTRATION(ACheckBox, {
    virtual _<AView> instanciate() {
        return _new<ACheckBox>("Гал Очка");
    }
})
DESIGNER_REGISTRATION(ARadioButton, {
    virtual _<AView> instanciate() {
        return _new<ARadioButton>("Флаж Очек");
    }
})
DESIGNER_REGISTRATION(AScrollbar, {
    virtual _<AView> instanciate() {
        return _new<AScrollbar>(LayoutDirection::HORIZONTAL);
    }
})


AUI_ENTRY
{
    Stylesheet::inst().load(AUrl(":style.less").open());
    _new<DesignerWindow>()->show();

	return 0;
}
