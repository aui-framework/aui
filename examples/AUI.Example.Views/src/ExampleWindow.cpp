#include <AUI/View/ARadioButton.h>
#include <AUI/View/ARadioGroup.h>
#include <AUI/Model/AListModel.h>
#include "ExampleWindow.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/View/AButton.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Platform/CustomCaptionWindow.h"
#include "AUI/View/ALabel.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/View/ASpacer.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include <AUI/Model/AListModel.h>
#include <AUI/View/AComboBox.h>

void fillWindow(_<AWindow> w)
{
	_<AViewContainer> t;
	if (w->getLayout() == nullptr)
	{
		t = w;
	} else
	{
		t = _new<AViewContainer>();
		w->addView(t);
	}
	t->setExpanding({ 1, 1 });
	t->setLayout(_new<AStackedLayout>());
	t->addView(_new<ALabel>(u8"Контент окна"));
}

ExampleWindow::ExampleWindow(): AWindow(u8"Примеры")
{
	setLayout(_new<AVerticalLayout>());

	addView(_new<ALabel>(u8"Давай я тебе расскажу, что ты не прав, когда говоришь, что на чистом С++ невозможно делать красивые программы"));

	auto horizontal = _new<AViewContainer>();
	horizontal->addCssName(".contents");
	horizontal->setLayout(_new<AHorizontalLayout>());
	addView(horizontal);

	horizontal->setExpanding({ 1, 1 });
	
	{
		auto c = _new<AViewContainer>(); 
		c->setLayout(_new<AVerticalLayout>());

		// кнопки
		{
			c->addView(_new<ALabel>(u8"Кнопки"));

			auto button = _new<AButton>(u8"Обычная кнопка");
			auto def = _new<AButton>(u8"Дефолтная кнопка");
			def->setDefault();
			auto disabled = _new<AButton>(u8"Неактивная кнопка");
			disabled->setEnabled(false);

			c->addView(button);
			c->addView(def);
			c->addView(disabled);
		}
		// Флажки
		{
			c->addView(_new<ALabel>(u8"Флажки"));
			c->addView(_new<ACheckBox>(u8"Флажок снят"));
			auto checked = _new<ACheckBox>(u8"Флажок установлен");
			checked->setChecked(true);
			c->addView(checked);
			auto disabled = _new<ACheckBox>(u8"Неактивный флажок");
			disabled->setDisabled();
			c->addView(disabled);
		}
		{
			c->addView(_new<ALabel>(u8"Радио кнопка"));
			c->addView(_new<ARadioGroup>(_new<AListModel<AString>>(AVector<AString>{
			    "Радио 1",
			    "Радио 2",
			    "Радио 3",
			    "Неактивная кнопка",
			})) by(ARadioGroup, {
			    getViews()[3]->setDisabled();
			}));
		}
        {
            c->addView(_new<AComboBox>(_new<AListModel<AString>>(AVector<AString>{
                "Комбобокс 1",
                "Комбобокс 2",
                "Комбобокс 3",
                "Комбобокс 4",
                "Комбобокс 5",
                "Комбобокс 6",
            })));
            c->addView(_new<AComboBox>(_new<AListModel<AString>>(AVector<AString>{
                "Отключённый комбобокс"
            })) by(AComboBox, {
                setDisabled();
            }));
        }
		
		horizontal->addView(c);
	}

	
	// окна
	{
		auto c = _new<AViewContainer>();
		c->setLayout(_new<AVerticalLayout>());
		
		c->addView(_new<ALabel>(u8"Окна"));
		
		auto def = _new<AButton>(u8"Обычное окно");
		connect(def->clicked, this, [&]()
		{
			auto w = _new<AWindow>(u8"Диалоговое окно", 400, 300);
			fillWindow(w);
			w->show();
			mWindows << w;
		});
		auto dialog = _new<AButton>(u8"Диалоговое окно");
		connect(dialog->clicked, this, [&]()
		{
			auto w = _new<AWindow>(u8"Диалоговое окно", 400, 300);
			fillWindow(w);
			w->show();
			w->setWindowStyle(WS_DIALOG);
			mWindows << w;
		});
		auto modal = _new<AButton>(u8"Модальное окно");
		connect(modal->clicked, this, [&]()
		{
			auto w = _new<AWindow>(u8"Модальное окно", 400, 300, this, WS_DIALOG);
			fillWindow(w);
			w->show();
			mWindows << w;
		});
		
		auto customWindowWithCaption = _new<AButton>(u8"Кастомное окно с заголовком");
		connect(customWindowWithCaption->clicked, this, [&]()
		{
			auto w = _new<CustomCaptionWindow>(u8"Кастомное окно", 400, 300);
			fillWindow(w);
			w->show();
			w->setWindowStyle(WS_DIALOG);
			mWindows << w;
		});
		
		auto customWindow = _new<AButton>(u8"Кастомное окно без заголовка");
		connect(customWindow->clicked, this, [&]()
		{
			auto w = _new<CustomWindow>(u8"Кастомное окно", 400, 300);
			fillWindow(w);
			w->show();
			w->setWindowStyle(WS_DIALOG);
			mWindows << w;
		});
		
		auto closeAll = _new<AButton>(u8"Закрыть все окна");
		connect(closeAll->clicked, this, [&]()
		{
			for (auto& w : mWindows)
				w->close();
			mWindows.clear();
		});

		c->addView(def);
		c->addView(dialog);
		c->addView(customWindowWithCaption);
		c->addView(customWindow);
		c->addView(modal);
		c->addView(closeAll);

		horizontal->addView(c);
	}

	// текстовые поля
	{
		auto c = _new<AViewContainer>();
		c->setLayout(_new<AVerticalLayout>());

		c->addView(_new<ALabel>(u8"Текстовые поля"));

		c->addView(_new<ALabel>(u8"Обычное поле"));
		c->addView(_new<ATextField>());

		c->addView(_new<ALabel>(u8"Числовое поле"));
		c->addView(_new<ANumberPicker>()); 

		horizontal->addView(c);
	}

	horizontal->addView(_new<ALabel>(u8"Сжать этот диск для экономии пространства"));

	
	addView(_container<AHorizontalLayout>({
		_new<ASpacer>(),
		_new<ALabel>(u8"\u00a9 Alex2772 2020, alex2772.ru")
					(&AView::setEnabled, false)
	}));
}
