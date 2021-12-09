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
#include <AUI/Platform/Platform.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Platform/AWindowManager.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/ABaseWindow.h>
#include <AUI/Platform/ACustomWindow.h>

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>
#include <AUI/Traits/arrays.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Devtools/DevtoolsPanel.h>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Render/OpenGLRenderer.h>

#include <AUI/Util/Cache.h>
#include <AUI/Util/AError.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>
#include <AUI/Platform/AMessageBox.h>

AWindow::Context::~Context() {
}

bool AWindow::isRenderingContextAcquired() {
    return true;
}

void AWindow::quit() {
    getWindowManager().mWindows.remove(shared_from_this());


    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::windowNativePreInit(const AString& name, int width, int height, AWindow* parent, WindowStyle ws) {

}

AWindow::~AWindow() {
}

extern unsigned char stencilDepth;

using namespace std::chrono;
using namespace std::chrono_literals;


static auto _gLastFrameTime = 0ms;


void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
}


void AWindow::updateDpi() {
    emit dpiChanged;

    mDpiRatio = 1.f;
    onDpiChanged();
}
void AWindow::redraw() {
    if (mUpdateLayoutFlag) {
        mUpdateLayoutFlag = false;
        updateLayout();
    }

    {

        // fps restriction
        {
            auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
            auto delta = now - _gLastFrameTime;
            // restriction 16ms = up to 60 frames per second
            const auto FRAME_DURATION = 16ms;

            if (FRAME_DURATION > delta) {
                std::this_thread::sleep_for(FRAME_DURATION - delta);
            }
            _gLastFrameTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
        }

#if !(AUI_PLATFORM_APPLE)
        painter p(mHandle);
#endif
        GL::State::activeTexture(0);
        GL::State::bindTexture(GL_TEXTURE_2D, 0);
        GL::State::bindVertexArray(0);
        GL::State::useProgram(0);

        Render::setWindow(this);
        glViewport(0, 0, getWidth(), getHeight());

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glClearColor(1.f, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // stencil
        glClearStencil(0);
        glStencilMask(0xff);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00);
        stencilDepth = 0;
        glStencilFunc(GL_EQUAL, 0, 0xff);

        doDrawWindow();


#if AUI_PLATFORM_WIN
        SwapBuffers(p.mHdc);
#elif AUI_PLATFORM_ANDROID

#elif AUI_PLATFORM_APPLE
        // TODO apple
#else
        glXSwapBuffers(gDisplay, mHandle);
#endif
    }

    emit redrawn();
}
void AWindow::restore() {
}

void AWindow::minimize() {
}

bool AWindow::isMinimized() const {
}


bool AWindow::isMaximized() const {
}

void AWindow::maximize() {
}

glm::ivec2 AWindow::getWindowPosition() const {
    return {0, 0};
}

void AWindow::flagRedraw() {
    if (mRedrawFlag) {
        mRedrawFlag = false;
    }
}


void AWindow::setSize(int width, int height) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, width, height);
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize(width, height);
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
    return position;
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
    return position;
}

void AWindow::setIcon(const AImage& image) {
    assert(image.getFormat() & AImage::BYTE);

}

void AWindow::hide() {
}


void AWindowManager::notifyProcessMessages() {

}


void AWindowManager::loop() {

}
