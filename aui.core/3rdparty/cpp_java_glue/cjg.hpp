// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2022 Alex2772
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