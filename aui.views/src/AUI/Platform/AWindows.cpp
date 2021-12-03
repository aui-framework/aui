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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */


#include "AUI/GL/gl.h"
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/Render.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "Platform.h"
#include "AMessageBox.h"
#include "AWindowManager.h"
#include "ADesktop.h"
#include "ABaseWindow.h"
#include "ACustomWindow.h"

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>
#include <AUI/Traits/arrays.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>

AWindow::Context AWindow::context = {};

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Devtools/DevtoolsPanel.h>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Render/OpenGLRenderer.h>



bool AWindow::consumesClick(const glm::ivec2& pos) {
    return AViewContainer::consumesClick(pos);
}

void AWindow::onClosed() {
    quit();
}

void AWindow::doDrawWindow() {
    render();
}

void AWindow::createDevtoolsWindow() {
    class DevtoolsWindow: public AWindow {
    public:
        DevtoolsWindow(): AWindow("Devtools", 500_dp, 400_dp) {}

    protected:
        void createDevtoolsWindow() override {
            // stub
        }
    };
    auto window = _new<DevtoolsWindow>();
    ALayoutInflater::inflate(window, _new<DevtoolsPanel>(this));
    window->show();
}

extern unsigned char stencilDepth;

using namespace std::chrono;
using namespace std::chrono_literals;


static auto _gLastFrameTime = 0ms;

_<AWindow> AWindow::wrapViewToWindow(const _<AView>& view, const AString& title, int width, int height, AWindow* parent, WindowStyle ws) {
    view->setExpanding();

    auto window = _new<AWindow>(title, width, height, parent, ws);
    window->setContents(Stacked {
        view
    });
    return window;
}

bool AWindow::isRedrawWillBeEfficient() {
    auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    auto delta = now - _gLastFrameTime;
    return 8ms < delta;
}


void AWindow::close() {
    onClosed();
}


void AWindow::onFocusAcquired() {
    mIsFocused = true;
    AViewContainer::onFocusAcquired();
}

void AWindow::onMouseMove(glm::ivec2 pos) {
    ABaseWindow::onMouseMove(pos);
}

void AWindow::onFocusLost() {
    mIsFocused = false;
    ABaseWindow::onFocusLost();
    if (AMenu::isOpen()) {
        AMenu::close();
    }
}

void AWindow::onKeyRepeat(AInput::Key key) {
    if (auto v = getFocusedView())
        v->onKeyRepeat(key);
}

ABaseWindow* AWindow::current() {
    return currentWindowStorage();
}

bool AWindow::shouldDisplayHoverAnimations() {
    return current()->isFocused() && !AInput::isKeyDown(AInput::LButton)
                                  && !AInput::isKeyDown(AInput::CButton)
                                  && !AInput::isKeyDown(AInput::RButton);
}


void AWindow::flagUpdateLayout() {
    flagRedraw();
    mUpdateLayoutFlag = true;
}

void AWindow::show() {
    if (!getWindowManager().mWindows.contains(shared_from_this())) {
        getWindowManager().mWindows << shared_from_this();
    }
    try {
        mSelfHolder = shared_from_this();
    } catch (...) {
        mSelfHolder = nullptr;
    }
    AThread::current() << [&]() {
        redraw();
    };
#if AUI_PLATFORM_WIN
    UpdateWindow(mHandle);
    ShowWindow(mHandle, SW_SHOWNORMAL);
#endif
    emit shown();
}

void AWindow::onCloseButtonClicked() {
    emit closed();
}


void AWindow::setPosition(const glm::ivec2& position) {
    setGeometry(position.x, position.y, getWidth(), getHeight());
}


glm::ivec2 AWindow::mapPositionTo(const glm::ivec2& position, _<AWindow> other) {
    return other->mapPosition(unmapPosition(position));
}


_<AView> AWindow::determineSharedPointer() {
    return (shared_from_this());
}



AWindowManager::AWindowManager(): mHandle(this) {}

AWindowManager::~AWindowManager() {

}


_<AOverlappingSurface> AWindow::createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) {
    class AOverlappingWindow: public AWindow {
    public:
        AOverlappingWindow(AWindow* parent):
        AWindow("MENU", 100, 100, parent, WindowStyle::SYS) {
            setCustomAss({ ass::Padding { 0 } });
        }
    };
    auto window = _new<AOverlappingWindow>(this);
    auto finalPos = unmapPosition(position);
    window->setGeometry(finalPos.x, finalPos.y, size.x, size.y);
    // show later
    window->show();

    auto surface = _new<AOverlappingSurface>();
    ALayoutInflater::inflate(window, surface);

    return surface;
}

void AWindow::closeOverlappingSurfaceImpl(AOverlappingSurface* surface) {
    if (auto c = dynamic_cast<AWindow*>(surface->getParent())) {
        c->close();
    }
}
