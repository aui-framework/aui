//
// Created by alex2772 on 9/12/20.
//

#if defined(__ANDROID__)

#include <jni.h>
#include <AUI/Logging/ALogger.h>
#include "OSAndroid.h"
#include "Entry.h"

JavaVM* _gVM;

//_<AThread> _gMainThread;


AAndroid::Ref _gClassAUI;
AAndroid::Ref _gClassMyGLSurfaceView;


int(*_gEntry)(const AStringVector&);

AUI_EXPORT int aui_main(JavaVM* vm, int(*aui_entry)(const AStringVector&)) {
    _gVM = vm;
    _gEntry = aui_entry;
    /*
    _gMainThread = _new<AThread>([aui_entry]() {
        ALogger::info("Application exited with exit code " + AString::number(aui_entry({})));
    });
    _gMainThread->start();*/

    return 0;
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
    auto methodGetDpiRatio = j->GetStaticMethodID(_gClassAUI.clazz(), "getDpiRatio", "()F");
    return j->CallStaticFloatMethod(_gClassAUI.clazz(), methodGetDpiRatio);
}

void AAndroid::requestRedraw() {
    auto j = getJNI();
    auto methodGetDpiRatio = j->GetStaticMethodID(_gClassMyGLSurfaceView.clazz(), "requestRedraw", "()V");
    j->CallStaticVoidMethod(_gClassMyGLSurfaceView.clazz(), methodGetDpiRatio);
}


extern "C"
JNIEXPORT void JNICALL
Java_ru_alex2772_aui_MyGLRenderer_handleInit(JNIEnv *env, jclass clazz) {
    //replace with one of your classes in the line below

    _gClassAUI = env->FindClass("ru/alex2772/aui/AUI");
    _gClassMyGLSurfaceView = env->FindClass("ru/alex2772/aui/MyGLSurfaceView");


    _gEntry({});
}


AAndroid::Ref &AAndroid::Ref::operator=(jobject object) {
    mObject = object ? AAndroid::getJNI()->NewGlobalRef(object) : nullptr;
    return *this;
}

#endif