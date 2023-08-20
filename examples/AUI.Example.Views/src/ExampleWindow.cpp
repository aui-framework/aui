//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
#include "AUI/View/ADoubleNumberPicker.h"
#include "AUI/View/ASpacerExpanding.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "DemoListModel.h"
#include "DemoTreeModel.h"
#include "AUI/View/ASpinner.h"
#include "DemoGraphView.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/View/ADragNDropView.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/View/ASlider.h"
#include "AUI/Platform/APlatform.h"
#include <AUI/Model/AListModel.h>
#include <AUI/View/ADropdownList.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/ASS/ASS.h>
#include <AUI/View/ASelectableLabel.h>
#include <AUI/View/AListView.h>
#include <AUI/View/ATextArea.h>
#include <AUI/View/ARulerView.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/View/ARulerArea.h>
#include <AUI/View/ATreeView.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/View/ADragArea.h>
#include <random>
#include <AUI/View/ASplitter.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ATabView.h>
#include <AUI/View/AGridSplitter.h>
#include <AUI/View/AText.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Traits/platform.h>
#include <dlfcn.h>

using namespace declarative;

struct MyModel {
    AColor color;
};

void fillWindow(_<AViewContainer> t)
{
    t->setLayout(_new<AStackedLayout>());
    t->addView(_new<ALabel>("Window contents"));
}


class AllViewsWindow: public AWindow {
public:
    AllViewsWindow(): AWindow("All views", 300_dp, 500_dp, nullptr, WindowStyle::MODAL) {
        setContents(Centered {
            AScrollArea::Builder().withContents(Vertical {
                Centered { _new<ALabel>("ALabel") },
                Centered { _new<AButton>("AButton") },
                Centered { _new<ATextField>() let { it->setText("ATextField"); } },
            } << ".all_views_wrap").withExpanding()
        });
    }
};

ExampleWindow::ExampleWindow(): AWindow("Examples", 800_dp, 700_dp)
{
    setContents(Horizontal {
        AScrollArea::Builder().withContents(
                Vertical{
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                }
                ).withExpanding(),
        AScrollArea::Builder().withContents(
                Vertical{
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                        AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style{
                                TextAlign::JUSTIFY},
                }
                ).withExpanding(),
    });
}

bool ExampleWindow::onDragEnter(const ADragNDrop::EnterEvent& event) {
    return true;
}

void ExampleWindow::onDragDrop(const ADragNDrop::DropEvent& event) {
    ABaseWindow::onDragDrop(event);
}
