/*
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
    AUI_NULLSAFE(AWindow::current())->setSize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleMouseButtonDown(JNIEnv *env, jclass clazz, jint x,
                                                           jint y) {
    AUI_NULLSAFE(AWindow::current())->onMousePressed({x, y}, AInput::LBUTTON);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleMouseButtonUp(JNIEnv *env, jclass clazz, jint x,
                                                           jint y) {
    AUI_NULLSAFE(AWindow::current())->onMouseReleased({x, y}, AInput::LBUTTON);
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
