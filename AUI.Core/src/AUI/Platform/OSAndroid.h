#pragma once
#include <jni.h>


namespace AAndroid {
    JNIEnv* getJNI();
    float getDpiRatio();
    void requestRedraw();

    class Ref {
    private:
        jobject mObject;

    public:
        Ref():
            mObject(nullptr) {

        }
        Ref(const jobject object):
            mObject(object ? AAndroid::getJNI()->NewGlobalRef(object) : nullptr) {

        }
        Ref(const Ref& object):
            mObject(object.mObject ? AAndroid::getJNI()->NewGlobalRef(object.mObject) : nullptr) {

        }
        ~Ref() {
            if (mObject)
                AAndroid::getJNI()->DeleteGlobalRef(mObject);
        }

        Ref& operator=(jobject object);


        jobject object() const {
            return mObject;
        }
        jclass clazz() const {
            return (jclass)mObject;
        }
    };
};

