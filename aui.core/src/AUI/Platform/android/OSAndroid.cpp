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


#include <AUI/Logging/ALogger.h>
#include "OSAndroid.h"
#include <AUI/Platform/Entry.h>
#include <unistd.h>
#include <AUI/Common/AByteBuffer.h>

JavaVM* _gVM;

//_<AThread> _gMainThread;


AAndroid::Ref _gClassAUI;


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
void AAndroid::openUrl(const AString& url) {
    auto j = getJNI();
    if (!_gClassAUI) {
        _gClassAUI = j->FindClass("com/github/aui/android/AUI");
        if (!_gClassAUI) {
            return;
        }
    }
    auto methodOpenUrl = j->GetStaticMethodID(_gClassAUI.clazz(), "openUrl", "(Ljava/lang/String;)V");
    std::u16string utf16 = { url.begin(), url.end() }; // TODO
    j->CallStaticVoidMethod(_gClassAUI.clazz(), methodOpenUrl, j->NewString(reinterpret_cast<const jchar*>(utf16.c_str()), utf16.size()));
}

void AAndroid::requestRedraw() {
    if (auto j = getJNI()) {
        if (auto methodGetDpiRatio = j->GetStaticMethodID(_gClassAUI.clazz(), "requestRedraw", "()V")) {
            j->CallStaticVoidMethod(_gClassAUI.clazz(), methodGetDpiRatio);
        }
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_MyGLSurfaceView_handleInit(JNIEnv *env, jclass clazz, jstring internalStoragePathR) {
    //replace with one of your classes in the line below

    _gClassAUI = env->FindClass("com/github/aui/android/AUI");

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

void AAndroid::showVirtualKeyboard() {
    if (auto j = getJNI()) {
        if (auto methodGetDpiRatio = j->GetStaticMethodID(_gClassAUI.clazz(), "showKeyboard", "()V")) {
            j->CallStaticVoidMethod(_gClassAUI.clazz(), methodGetDpiRatio);
        }
    }
}

void AAndroid::hideVirtualKeyboard() {
    if (auto j = getJNI()) {
        if (auto methodGetDpiRatio = j->GetStaticMethodID(_gClassAUI.clazz(), "hideKeyboard", "()V")) {
            j->CallStaticVoidMethod(_gClassAUI.clazz(), methodGetDpiRatio);
        }
    }
}
