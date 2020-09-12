//
// Created by alex2772 on 9/12/20.
//

#if defined(__ANDROID__)

#include <jni.h>
#include "OSAndroid.h"
#include "Entry.h"

AUI_IMPORT int aui_entry(const AStringVector& args);

extern "C"
JNIEXPORT void JNICALL
Java_ru_alex2772_aui_AUI_entry(JNIEnv *env, jclass clazz) {
    aui_entry({});
}

#endif