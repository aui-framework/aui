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

//
// Created by alex2 on 5/21/2021.
//


#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AImageView.h>
#include "WelcomePage.h"
#include <AUI/ASS/ASS.h>

using namespace ass;

void WelcomePage::inflate(const _<AViewContainer>& container, const InstallerModel& model) {
    container->setLayout(_new<AStackedLayout>());
    container->addView(
        Horizontal {
            _new<AImageView>(AUrl(":img/side.png")) with_style { FixedSize { 100_dp, 300_dp } },
            Vertical {
                _new<ALabel>("aui.installer.welcome_title") << ".title",
                _new<ALabel>("aui.installer.welcome_desc")
            } let {
                it->setExpanding({2, 2});
                it->setCustomAss(Padding { 8_dp });
            }
        } let { it->setExpanding({2, 2});});
}
