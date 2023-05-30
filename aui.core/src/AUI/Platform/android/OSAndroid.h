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

#pragma once
#include <jni.h>


namespace AAndroid {
    JNIEnv* getJNI();
    void setJavaVM(JavaVM* vm);
    float getDpiRatio();
    void openUrl(const AString& url);
    void requestRedraw();
    void showVirtualKeyboard();
    void hideVirtualKeyboard();

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

        bool operator!() const {
            return !mObject;
        }

        operator bool() const {
            return mObject;
        }


        jobject object() const {
            return mObject;
        }
        jclass clazz() const {
            return (jclass)mObject;
        }
    };
};

