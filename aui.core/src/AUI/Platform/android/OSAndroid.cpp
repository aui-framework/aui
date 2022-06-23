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
#include <AUI/Logging/ALogger.h>
#include "OSAndroid.h"
#include <AUI/Platform/Entry.h>
#include <unistd.h>

JavaVM* _gVM;

//_<AThread> _gMainThread;


AAndroid::Ref _gClassAUI;
AAndroid::Ref _gClassMyGLSurfaceView;


int(*_gEntry)(AStringVector);

AUI_EXPORT int aui_main(JavaVM* vm, int(*aui_entry)(AStringVector)) {
    _gVM = vm;
    _gEntry = aui_entry;
    /*
    _gMainThread = _new<AThread>([aui_entry]() {
        ALogger::info("Application exited with exit code " + AString::number(aui_entry({})));
    });
    _gMainThread->start();*/

    return 0;
}

void AAndroid::setJavaVM(JavaVM* vm) {
    _gVM = vm;
}
JNIEnv* AAndroid::getJNI() {
    JNIEnv* env;

    struct attacher {
        attacher() {
            JNIEnv* env;
            _gVM->AttachCurrentThread(&env, nullptr);
        }
    };
    thread_local attacher a;

    _gVM->GetEnv((void**)&env, JNI_VERSION_1_2);
    assert(env);

    return env;
}

float AAndroid::getDpiRatio() {
    auto j = getJNI();
    if (!j) return 1.f;
    if (!_gClassAUI) {
        _gClassAUI = j->FindClass("com/github/aui/android/AUI");
        if (!_gClassAUI) {
            return 1.f;
        }
    }
    auto methodGetDpiRatio = j->GetStaticMethodID(_gClassAUI.clazz(), "getDpiRatio", "()F");
    return j->CallStaticFloatMethod(_gClassAUI.clazz(), methodGetDpiRatio);
}

void AAndroid::requestRedraw() {
    if (auto j = getJNI()) {
        if (auto methodGetDpiRatio = j->GetStaticMethodID(_gClassMyGLSurfaceView.clazz(), "requestRedraw", "()V")) {
            j->CallStaticVoidMethod(_gClassMyGLSurfaceView.clazz(), methodGetDpiRatio);
        }
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleInit(JNIEnv *env, jclass clazz, jstring internalStoragePathR) {
    //replace with one of your classes in the line below

    _gClassAUI = env->FindClass("com/github/aui/android/AUI");
    _gClassMyGLSurfaceView = env->FindClass("com/github/aui/android/MyGLSurfaceView");

    jboolean isCopy;
    auto internalStoragePath = env->GetStringUTFChars(internalStoragePathR, &isCopy);
    chdir(internalStoragePath);
    env->ReleaseStringUTFChars(internalStoragePathR, internalStoragePath);

    _gEntry({});
}


AAndroid::Ref &AAndroid::Ref::operator=(jobject object) {
    mObject = object ? AAndroid::getJNI()->NewGlobalRef(object) : nullptr;
    return *this;
}
