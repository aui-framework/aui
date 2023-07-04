// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.


#include <jni.h>
#include <AUI/Thread/AThread.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Logging/ALogger.h>

static void runOnGLThread(std::function<void()> callback) {
    AUI_NULLSAFE(AWindow::current())->getThread()->enqueue(std::move(callback));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleRedraw(JNIEnv *env, jclass clazz) {
    AUI_NULLSAFE(AThread::current()->getCurrentEventLoop())->loop();
    AUI_NULLSAFE(dynamic_cast<AWindow*>(AWindow::current()))->AWindow::redraw();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleResize(JNIEnv *env, jclass clazz, jint width, jint height) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->setSize({width, height});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleLongPress(JNIEnv *env, jclass clazz, jint x, jint y) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onGesture({x, y}, ALongPressEvent{});
    });
}

static glm::ivec2 gestureOriginPos{0, 0};
static glm::ivec2 scrollPrevValue{0, 0};

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleMouseButtonDown(JNIEnv *env, jclass clazz, jint x,
                                                          jint y) {
    scrollPrevValue = gestureOriginPos = {x, y};
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerPressed({{x, y}, AInput::LBUTTON});
    });
}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleMouseButtonUp(JNIEnv *env, jclass clazz, jint x,
                                                        jint y) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerReleased({{x, y}, AInput::LBUTTON});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleMouseMove(JNIEnv *env, jclass clazz, jint x,
                                                    jint y) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerMove({x, y});
    });
}


extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleKineticScroll(JNIEnv *env, jclass clazz, jint x, jint y) {
    glm::ivec2 current = {x, y};

    if (current == glm::ivec2(0)) return;

    auto delta = scrollPrevValue - current;
    scrollPrevValue = current;
    AUI_NULLSAFE(AWindow::current())->onGesture(gestureOriginPos,
                                                AFingerDragEvent{
        .delta = delta,
        .kinetic = true,
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleScroll(JNIEnv *env, jclass clazz,
                                                 jint originX,
                                                 jint originY,
                                                 jfloat velX,
                                                 jfloat velY) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onGesture({originX, originY}, AFingerDragEvent{
            .delta = {velX, velY}
        });
    });
}
