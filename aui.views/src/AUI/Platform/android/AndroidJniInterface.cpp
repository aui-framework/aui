/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


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
Java_com_github_aui_android_AUIView_handleRedraw__(JNIEnv *env, jclass clazz) {
    AUI_NULLSAFE(AThread::current()->getCurrentEventLoop())->loop();
    AUI_NULLSAFE(dynamic_cast<AWindow*>(AWindow::current()))->AWindow::redraw();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AUIView_handleResize__II(JNIEnv *env, jclass clazz, jint width, jint height) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->setSize({width, height});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AUIView_handleLongPress__II(JNIEnv *env, jclass clazz, jfloat x, jfloat y) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onGesture({x, y}, ALongPressEvent{});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AUIView_handlePointerButtonDown__FFI(JNIEnv *env, jclass clazz, jfloat x,
                                                                 jfloat y, jint pointerId) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerPressed({{x, y}, APointerIndex::finger(pointerId)});
    });
}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AUIView_handlePointerButtonUp__FFI(JNIEnv *env, jclass clazz, jfloat x,
                                                               jfloat y, jint pointerId) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerReleased({{x, y}, APointerIndex::finger(pointerId)});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AUIView_handlePointerMove__FFI(JNIEnv *env, jclass clazz, jfloat x,
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
