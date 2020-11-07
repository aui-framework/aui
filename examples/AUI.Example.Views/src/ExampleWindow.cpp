#include <AUI/View/ARadioButton.h>
#include <AUI/View/ARadioGroup.h>
#include <AUI/Model/AListModel.h>
#include "ExampleWindow.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/View/AButton.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Platform/ACustomCaptionWindow.h"
#include "AUI/View/ALabel.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/View/ASpacer.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include <AUI/Model/AListModel.h>
#include <AUI/View/AComboBox.h>
#include <AUI/i18n/i18n.h>

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
			c->addView(_new<ALabel>("Кнопки"_i18n));

			auto button = _new<AButton>(u8"Обычная кнопка"_i18n);
			auto def = _new<AButton>(u8"Дефолтная кнопка"_i18n);
			def->setDefault();
			auto disabled = _new<AButton>(u8"Неактивная кнопка"_i18n);
			disabled->setEnabled(false);

			c->addView(button);
			c->addView(def);
			c->addView(disabled);
		}
		// Флажки
		{
			c->addView(_new<ALabel>(u8"Флажки"_i18n));
			c->addView(_new<ACheckBox>(u8"Флажок снят"_i18n));
			auto checked = _new<ACheckBox>(u8"Флажок установлен"_i18n);
			checked->setChecked(true);
			c->addView(checked);
			auto disabled = _new<ACheckBox>(u8"Неактивный флажок"_i18n);
			disabled->setDisabled();
			c->addView(disabled);
		}
		{
			c->addView(_new<ALabel>(u8"Радио кнопка"_i18n));
			c->addView(_new<ARadioGroup>(_new<AListModel<AString>>(AVector<AString>{
			    "Радио 1"_i18n,
			    "Радио 2"_i18n,
			    "Радио 3"_i18n,
			    "Неактивная кнопка"_i18n,
			})) let (ARadioGroup, {
			    getViews()[3]->setDisabled();
			}));
		}
        {
            c->addView(_new<AComboBox>(_new<AListModel<AString>>(AVector<AString>{
                "Комбобокс 1"_i18n,
                "Комбобокс 2"_i18n,
                "Комбобокс 3"_i18n,
                "Комбобокс 4"_i18n,
                "Комбобокс 5"_i18n,
                "Комбобокс 6"_i18n,
            })));
            c->addView(_new<AComboBox>(_new<AListModel<AString>>(AVector<AString>{
                "Отключённый комбобокс"_i18n
            })) let(AComboBox, {
                setDisabled();
            }));
        }
		
		horizontal->addView(c);
	}

	
	// окна
	{
		auto c = _new<AViewContainer>();
		c->setLayout(_new<AVerticalLayout>());
		
		c->addView(_new<ALabel>(u8"Окна"_i18n));
		
		auto def = _new<AButton>(u8"Обычное окно"_i18n);
		connect(def->clicked, this, [&]()
		{
			auto w = _new<AWindow>(u8"Диалоговое окно"_i18n, 400, 300);
			fillWindow(w);
			w->show();
			mWindows << w;
		});
		auto dialog = _new<AButton>(u8"Диалоговое окно"_i18n);
		connect(dialog->clicked, this, [&]()
		{
			auto w = _new<AWindow>(u8"Диалоговое окно"_i18n, 400, 300);
			fillWindow(w);
			w->show();
			w->setWindowStyle(WS_DIALOG);
			mWindows << w;
		});
		auto modal = _new<AButton>(u8"Модальное окно"_i18n);
		connect(modal->clicked, this, [&]()
		{
			auto w = _new<AWindow>(u8"Модальное окно"_i18n, 400, 300, this, WS_DIALOG);
			fillWindow(w);
			w->show();
			mWindows << w;
		});
		
		auto customWindowWithCaption = _new<AButton>(u8"Кастомное окно с заголовком"_i18n);
		connect(customWindowWithCaption->clicked, this, [&]()
		{
			auto w = _new<ACustomCaptionWindow>(u8"Кастомное окно"_i18n, 400, 300);
			fillWindow(w);
			w->show();
			//w->setWindowStyle(WS_DIALOG);
			mWindows << w;
		});
		
		auto customWindow = _new<AButton>(u8"Кастомное окно без заголовка"_i18n);
		connect(customWindow->clicked, this, [&]()
		{
			auto w = _new<ACustomWindow>(u8"Кастомное окно"_i18n, 400, 300);
			fillWindow(w);
			w->show();
			w->setWindowStyle(WS_DIALOG);
			mWindows << w;
		});
		
		auto closeAll = _new<AButton>(u8"Закрыть все окна"_i18n);
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

		c->addView(_new<ALabel>(u8"Текстовые поля"_i18n));

		c->addView(_new<ALabel>(u8"Обычное поле"_i18n));
		c->addView(_new<ATextField>());

		c->addView(_new<ALabel>(u8"Числовое поле"_i18n));
		c->addView(_new<ANumberPicker>()); 

		horizontal->addView(c);
	}

	addView(_container<AHorizontalLayout>({
		_new<ASpacer>(),
		_new<ALabel>(u8"\u00a9 Alex2772 2020, alex2772.ru")
					(&AView::setEnabled, false)
	}));
}
