/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
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
#include "DemoListModel.h"
#include <AUI/Model/AListModel.h>
#include <AUI/View/AComboBox.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/ASS/ASS.h>
#include <AUI/View/ASelectableLabel.h>
#include <AUI/View/AListView.h>
#include <AUI/View/ATextArea.h>
#include <AUI/View/ARulerView.h>
#include <AUI/View/AImageView.h>
#include <AUI/View/ARulerArea.h>

using namespace ass;

void fillWindow(_<AViewContainer> t)
{
	t->setLayout(_new<AStackedLayout>());
	t->addView(_new<ALabel>("Window contents"));
}

ExampleWindow::ExampleWindow(): AWindow("Examples")
{
	setLayout(_new<AVerticalLayout>());

	addView(_new<ASelectableLabel>("Building beautiful programs in pure C++ without chrome embded framework"));

	auto horizontal = _new<AViewContainer>();
    horizontal << ".contents";
	horizontal->setLayout(_new<AHorizontalLayout>());
	addView(horizontal);

	horizontal->setExpanding({ 1, 1 });
	
	{
	    horizontal->addView(Vertical {
	        // buttons
	        _new<ALabel>("Buttons"),
	        _new<AButton>("Common button"),
	        _new<AButton>("Default button") let { it->setDefault(); },
	        _new<AButton>("Disabled button") let { it->setDisabled(); },

	        // checkboxes
	        _new<ALabel>("Checkboxes"),
	        _new<ACheckBox>("Unchecked checkbox"),
	        _new<ACheckBox>("Checked checkbox") let { it->setChecked(true); },
	        _new<ACheckBox>("Disabled checkbox") let { it->setDisabled(); },

	        // radiobuttons
            _new<ALabel>("Radiobuttons"),
            _new<ARadioGroup>(_new<AListModel<AString>>({
                "Radiobutton 1",
                "Radiobutton 2",
                "Radiobutton 3",
                "Disabled radiobutton",
            })) let {
                it->getViews()[3]->setDisabled();
            },

            // comboboxes
            _new<ALabel>("Comboboxes"),
            _new<AComboBox>(_new<AListModel<AString>>({
                    "Combobox 1",
                    "Combobox 2",
                    "Combobox 3",
                    "Combobox 4",
                    "Combobox 5",
                    "Combobox 6",
            })),
            _new<AComboBox>(_new<AListModel<AString>>({"Disabled combobox"})) let { it->setDisabled(); }
	    });
	}

	
	// окна
	{
		horizontal->addView(Vertical {
		   _new<ALabel>("Windows"),
           _new<AButton>("Common window").connect(&AButton::clicked, this, [&] {
               auto w = _new<AWindow>("Common window", 400_dp, 300_dp);
               fillWindow(w);
               w->show();
               mWindows << w;
           }),
           _new<AButton>("Dialog window").connect(&AButton::clicked, this, [&] {
               auto w = _new<AWindow>("Dialog window", 400_dp, 300_dp);
               fillWindow(w);
               w->show();
               w->setWindowStyle(WindowStyle::DIALOG);
               mWindows << w;
           }),
           _new<AButton>("Modal window").connect(&AButton::clicked, this, [&] {
               auto w = _new<AWindow>("Modal window", 400_dp, 300_dp, this, WindowStyle::DIALOG);
               fillWindow(w);
               w->show();
               mWindows << w;
           }),
           _new<AButton>("Custom window with caption").connect(&AButton::clicked, this, [&] {
               auto w = _new<ACustomCaptionWindow>("Custom window with caption", 400_dp, 300_dp);
               fillWindow(w->getContentContainer());
               w->show();
               //w->setWindowStyle(WindowStyle::DIALOG);
               mWindows << w;
           }),
           _new<AButton>("Custom window without caption").connect(&AButton::clicked, this, [&] {
               auto w = _new<ACustomWindow>("Custom window without caption", 400_dp, 300_dp);
               fillWindow(w);
               w->show();
               w->setWindowStyle(WindowStyle::DIALOG);
               mWindows << w;
           }),
           _new<AButton>("Close all windows").connect(&AButton::clicked, this, [&] {
               for (auto& w : mWindows)
                   w->close();
               mWindows.clear();
           }),


           // list view
           _new<ALabel>("List view"),
           [] { // lambda style inlining
               auto model = _new<DemoListModel>();

               return Vertical {
                       Horizontal {
                           _new<AButton>("Add").connect(&AButton::clicked, slot(model)::addItem),
                           _new<AButton>("Remove").connect(&AButton::clicked, slot(model)::removeItem),
                           _new<ASpacer>(),
                       },
                       _new<AListView>(model)
               };
           }()
		});

	}

	{
		horizontal->addView(Vertical {
            // fields
		    _new<ALabel>("Fields"),
		    _new<ALabel>("Text field"),
		    _new<ATextField>(),
		    _new<ALabel>("Number picker"),
		    _new<ANumberPicker>(),
            _new<ALabel>("Text area"),
		    _new<ATextArea>("Copyright (c) 2021 Alex2772\n\n"
                      "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated "
                      "documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the "
                      "rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to "
                      "permit persons to whom the Software is furnished to do so, subject to the following conditions:\n\n"

                      "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the "
                      "Software.\n\n"
                      "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE "
                      "WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR "
                      "COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR "
                      "OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.) "),

            // Rulers
            _new<ALabel>("ARulerArea"),
            _new<ARulerArea>(_new<AImageView>(":img/example.jpg"_url) with_style { MinSize { 300_dp, {} } }),
		});
	}


	addView(Horizontal{
		_new<ASpacer>(),
        _new<ACheckBox>("Enabled") let {
            it->setChecked();
            connect(it->checked, slot(horizontal)::setEnabled);
        },
		_new<ALabel>("\u00a9 Alex2772, 2021, alex2772.ru") let {
		    it->setEnabled(false);
		}
	});
}
