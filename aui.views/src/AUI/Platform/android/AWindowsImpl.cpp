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

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>
#include <AUI/Traits/arrays.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>


#include <AUI/Platform/OSAndroid.h>
#include <AUI/Platform/Platform.h>
#include <AUI/Render/OpenGLRenderer.h>

struct painter {
private:
    jobject mHandle;

public:
    static thread_local bool painting;

    painter(jobject handle) :
            mHandle(handle) {
        assert(!painting);
        painting = true;
    }

    ~painter() {
        assert(painting);
        painting = false;
    }
};

thread_local bool painter::painting = false;


AWindow::Context::~Context() {

}


void AWindow::windowNativePreInit(const AString& name, int width, int height, AWindow* parent, WindowStyle ws) {
    mWindowTitle = name;
    mParentWindow = parent;

    currentWindowStorage() = this;

    connect(closed, this, &AWindow::close);


    ALogger::info((const char*) glGetString(GL_VERSION));
    ALogger::info((const char*) glGetString(GL_VENDOR));
    ALogger::info((const char*) glGetString(GL_RENDERER));
    ALogger::info((const char*) glGetString(GL_EXTENSIONS));

    do_once {
        Render::setRenderer(std::make_unique<OpenGLRenderer>());
    }

    //assert(glGetError() == 0);

    updateDpi();
    Render::setWindow(this);

    checkForStencilBits();

    setWindowStyle(ws);

}

AWindow::~AWindow() {
    // TODO close
}

extern unsigned char stencilDepth;

using namespace std::chrono;
using namespace std::chrono_literals;


static auto _gLastFrameTime = 0ms;


bool AWindow::isRenderingContextAcquired() {
    return painter::painting;
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

        painter p(mHandle);

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
    }

    emit redrawn();
}

void AWindow::quit() {
    getWindowManager().mWindows.remove(shared_from_this());

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
}

void AWindow::updateDpi() {
    emit dpiChanged;

    mDpiRatio = Platform::getDpiRatio();

    onDpiChanged();
}

void AWindow::restore() {

}

void AWindow::minimize() {

}

bool AWindow::isMinimized() const {
    return false;
}


bool AWindow::isMaximized() const {
    return true;
}

void AWindow::maximize() {

}

glm::ivec2 AWindow::getWindowPosition() const {
    return {0, 0};
}


void AWindow::flagRedraw() {
    AAndroid::requestRedraw();
}

void AWindow::setSize(int width, int height) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, width, height);
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize(width, height);
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
    return position - getWindowPosition();
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
    return position + getWindowPosition();
}


void AWindow::setIcon(const AImage& image) {

}

void AWindow::hide() {

}


void AWindowManager::notifyProcessMessages() {
    AAndroid::requestRedraw();
}

void AWindowManager::loop() {
    AThread::current()->processMessages();
    if (!mWindows.empty()) {
        mWindows.back()->redraw();
    }
}
