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
// Created by alex2 on 5/19/2021.
//


#include <AUI/Util/UIBuildingHelpers.h>
#include "MainWindow.h"
#include <Page/WelcomePage.h>

#include <AUI/ASS/ASS.h>
#include <Page/LicensePage.h>
#include <Page/InstallDirPage.h>

using namespace ass;

MainWindow::MainWindow():
        AWindow("Installer", 400_dp, 300_dp, nullptr, WS_NO_RESIZE)
{
    setContents(Vertical {
        Horizontal {

            Vertical {
                mContent = _new<AViewContainer>() let {
                    it->setExpanding({2, 2});
                },
                _new<ASpacer>(),
            } let {
                it->setCustomAss({
                    BackgroundSolid { 0xffffff_rgb },
                });
            }
        } let {
            it->setExpanding({2, 2});
        },
        Horizontal {
            _new<ASpacer>(),
            mBackButton = _new<AButton>("< Назад"),
            mNextButton = _new<AButton>("Вперёд >").connect(&AButton::clicked, this, [&] {
                setPage(mCurrentPageId + 1);
            }),
            _new<AButton>("Отмена") let {
                it->setCustomAss({
                    Margin { {}, {}, {}, 8_dp }
                });
            }
        } with_style { Padding { 8_dp } }
    });

    setCustomAss({
        Padding { 0_dp }
    });

    mPages.push_back(_new<WelcomePage>());
    mPages.push_back(_new<LicensePage>());
    mPages.push_back(_new<InstallDirPage>());

    setPage(0);

    pack();
}

void MainWindow::setPage(int pageId) {
    mCurrentPageId = pageId;
    mPages[pageId]->inflate(mContent, mInstallerModel);
    mBackButton->setEnabled(pageId != 0);
    mContent->updateLayout();
}
