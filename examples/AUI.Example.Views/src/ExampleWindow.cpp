/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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
#include <AUI/i18n/AI18n.h>
#include <AUI/View/ASelectableLabel.h>

void fillWindow(_<AViewContainer> t)
{
	t->setLayout(_new<AStackedLayout>());
	t->addView(_new<ALabel>("Window contents"));
}

ExampleWindow::ExampleWindow(): AWindow("Examples")
{
	setLayout(_new<AVerticalLayout>());

	// This dialog is constructed using the old way. The newer way is declarative ui.

	addView(_new<ASelectableLabel>("Building beautiful programs in pure C++ without chrome embded framework"));

	auto horizontal = _new<AViewContainer>();
    horizontal->addAssName(".contents");
	horizontal->setLayout(_new<AHorizontalLayout>());
	addView(horizontal);

	horizontal->setExpanding({ 1, 1 });
	
	{
		auto c = _new<AViewContainer>(); 
		c->setLayout(_new<AVerticalLayout>());

		// buttons
		{
			c->addView(_new<ALabel>("Buttons"));

			auto button = _new<AButton>("Common button");
			auto def = _new<AButton>("Default button");
			def->setDefault();
			auto disabled = _new<AButton>("Disabled button");
			disabled->setEnabled(false);

			c->addView(button);
			c->addView(def);
			c->addView(disabled);
		}
		// checkboxes
		{
			c->addView(_new<ALabel>("Checkboxes"));
			c->addView(_new<ACheckBox>("Unchecked checkbox"));
			auto checked = _new<ACheckBox>("Disabled checkbox");
			checked->setChecked(true);
			c->addView(checked);
			auto disabled = _new<ACheckBox>("Disable");
			disabled->setDisabled();
			c->addView(disabled);
		}

		// radiobuttons
		{
			c->addView(_new<ALabel>("Radiobuttons"));
			c->addView(_new<ARadioGroup>(_new<AListModel<AString>>(AVector<AString>{
			    "Radiobutton 1",
			    "Radiobutton 2",
			    "Radiobutton 3",
			    "Disabled radiobutton",
			})) let (ARadioGroup, {
			    getViews()[3]->setDisabled();
			}));
		}

		// comboboxes
        {
			c->addView(_new<ALabel>("Comboboxes"));
            c->addView(_new<AComboBox>(_new<AListModel<AString>>(AVector<AString>{
                "Combobox 1",
                "Combobox 2",
                "Combobox 3",
                "Combobox 4",
                "Combobox 5",
                "Combobox 6",
            })));
            c->addView(_new<AComboBox>(_new<AListModel<AString>>(AVector<AString>{
                "Disabled combobox"
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
		
		c->addView(_new<ALabel>("Windows"));
		
		auto def = _new<AButton>("Common window");
		connect(def->clicked, this, [&]()
		{
			auto w = _new<AWindow>("Common window", 400_dp, 300_dp);
			fillWindow(w);
			w->show();
			mWindows << w;
		});
		auto dialog = _new<AButton>("Dialog window");
		connect(dialog->clicked, this, [&]()
		{
			auto w = _new<AWindow>("Dialog window", 400_dp, 300_dp);
			fillWindow(w);
			w->show();
			w->setWindowStyle(WS_DIALOG);
			mWindows << w;
		});
		auto modal = _new<AButton>("Modal window");
		connect(modal->clicked, this, [&]()
		{
			auto w = _new<AWindow>("Modal window", 400_dp, 300_dp, this, WS_DIALOG);
			fillWindow(w);
			w->show();
			mWindows << w;
		});
		
		auto customWindowWithCaption = _new<AButton>("Custom window with caption");
		connect(customWindowWithCaption->clicked, this, [&]()
		{
			auto w = _new<ACustomCaptionWindow>("Custom window with caption", 400_dp, 300_dp);
			fillWindow(w->getContentContainer());
			w->show();
			//w->setWindowStyle(WS_DIALOG);
			mWindows << w;
		});
		
		auto customWindow = _new<AButton>("Custom window without caption");
		connect(customWindow->clicked, this, [&]()
		{
			auto w = _new<ACustomWindow>("Custom window without caption", 400_dp, 300_dp);
			fillWindow(w);
			w->show();
			w->setWindowStyle(WS_DIALOG);
			mWindows << w;
		});
		
		auto closeAll = _new<AButton>("Close all windows");
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

	// fields
	{
		auto c = _new<AViewContainer>();
		c->setLayout(_new<AVerticalLayout>());

		c->addView(_new<ALabel>("Fields"));

		c->addView(_new<ALabel>("Text field"));
		c->addView(_new<ATextField>());

		c->addView(_new<ALabel>("Number picker"));
		c->addView(_new<ANumberPicker>()); 

		horizontal->addView(c);
	}

	addView(_container<AHorizontalLayout>({
		_new<ASpacer>(),
		_new<ALabel>("\u00a9 Advanced Universal Interface 2020")
					(&AView::setEnabled, false)
	}));
}
