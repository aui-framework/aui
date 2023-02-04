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

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleRedraw(JNIEnv *env, jclass clazz) {
    if (auto el = AThread::current()->getCurrentEventLoop())
        el->loop();
    AUI_NULLSAFE(dynamic_cast<AWindow*>(AWindow::current()))->AWindow::redraw();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleResize(JNIEnv *env, jclass clazz, jint width, jint height) {
    AUI_NULLSAFE(AWindow::current())->setSize({width, height});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleMouseButtonDown(JNIEnv *env, jclass clazz, jint x,
                                                           jint y) {
    AUI_NULLSAFE(AWindow::current())->onMousePressed({ {x, y}, AInput::LBUTTON });
}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleMouseButtonUp(JNIEnv *env, jclass clazz, jint x,
                                                           jint y) {
    AUI_NULLSAFE(AWindow::current())->onMouseReleased({ {x, y}, AInput::LBUTTON });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleMouseMove(JNIEnv *env, jclass clazz, jint x,
                                                           jint y) {
    AUI_NULLSAFE(AWindow::current())->onMouseMove({x, y});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleScroll(JNIEnv *env, jclass clazz,
                                                            jint originX,
                                                            jint originY,
                                                            jfloat velX,
                                                            jfloat velY) {
    AUI_NULLSAFE(AWindow::current())->onGesture({originX, originY}, AFingerDragEvent{ {velX, velY} });
}
