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

#include "Globals.h"


namespace aui::jni {

    /**
     * @brief Global ref.
     * @ingroup jni
     */
    class GlobalRef {
    private:
        jobject mObject;

    public:
        GlobalRef():
                mObject(nullptr) {

        }
        GlobalRef(const jobject object):
                mObject(object ? aui::jni::env()->NewGlobalRef(object) : nullptr) {

        }
        GlobalRef(const GlobalRef& object):
                mObject(object.mObject ? aui::jni::env()->NewGlobalRef(object.mObject) : nullptr) {

        }
        ~GlobalRef() {
            if (mObject)
                aui::jni::env()->DeleteGlobalRef(mObject);
        }

        bool operator!() const {
            return !mObject;
        }

        operator bool() const {
            return mObject;
        }


        jobject asObject() const {
            return mObject;
        }
        jclass asClass() const {
            return (jclass)mObject;
        }
    };
}
