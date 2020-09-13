//
// Created by alex2772 on 9/12/20.
//

#if defined(__ANDROID__)

#include <jni.h>
#include "OSAndroid.h"
#include "Entry.h"

JavaVM* _gVM;
int(*_gEntry)(const AStringVector&);

AUI_EXPORT int aui_main(JavaVM* vm, int(*aui_entry)(const AStringVector&)) {
    _gVM = vm;
    _gEntry = aui_entry;
    return 0;
}

JNIEnv* AAndroid::getJNI() {
    JNIEnv* env;
    _gVM->GetEnv((void**)&env, JNI_VERSION_1_2);

    return env;
}

extern "C"
JNIEXPORT void JNICALL
Java_ru_alex2772_aui_MyGLRenderer_handleInit(JNIEnv *env, jclass clazz) {
    _gEntry({});
}

#endif
