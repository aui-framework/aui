#pragma once

#include <jni.h>

namespace cjg {
    JavaVM* vm();
    void set_vm(JavaVM* vm);

    class ref {
    private:
        jobject mObject;

    public:
        ref():
                mObject(nullptr) {

        }
        ref(const jobject object):
                mObject(object ? cjg::jni()->NewGlobalRef(object) : nullptr) {

        }
        ref(const ref& object):
                mObject(object.mObject ? cjg::jni()->NewGlobalRef(object.mObject) : nullptr) {

        }
        ~ref() {
            if (mObject)
                cjg::jni()->DeleteGlobalRef(mObject);
        }
    };
}