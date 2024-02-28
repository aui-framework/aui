// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include <unordered_map>

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
Java_com_github_aui_android_AuiView_handleLongPress(JNIEnv *env, jclass clazz, jfloat x, jfloat y) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onGesture({x, y}, ALongPressEvent{});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handlePointerButtonDown(JNIEnv *env, jclass clazz, jfloat x,
                                                          jfloat y, jint pointerId) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerPressed({{x, y}, APointerIndex::finger(pointerId)});
    });
}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handlePointerButtonUp(JNIEnv *env, jclass clazz, jfloat x,
                                                        jfloat y, jint pointerId) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerReleased({{x, y}, APointerIndex::finger(pointerId)});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handlePointerMove(JNIEnv *env, jclass clazz, jfloat x,
                                                    jfloat y, jint pointerId) {
    // android sends move events for all fingers even if just one finger moved.
    static std::unordered_map<jint /* pointerIndex */, glm::ivec2> prevValues;
    glm::ivec2 currentValue{x, y};
    if (prevValues[pointerId] == currentValue) {
        return;
    }
    prevValues[pointerId] = currentValue;

    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerMove(currentValue, {APointerIndex::finger(pointerId)});
    });
}
