#pragma once
#include <jni.h>


namespace AAndroid {
    JNIEnv* getJNI();

    float getDpiRatio();

    void requestRedraw();
};

