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

#include <AUI/Common/AObject.h>
#include <jni.h>


namespace aui::jni {

    /**
     * @return Java VM pointer.
     * @ingroup jni
     */
    extern "C" API_AUI_CORE JavaVM* javaVM();

    /**
     * @brief Sets global Java VM pointer.
     * @ingroup jni
     */
    extern "C" API_AUI_CORE void setJavaVM(JavaVM* vm);



    /**
     * @return JNI Env pointer for the current thread.
     * @ingroup jni
     */
    JNIEnv* env() {
        thread_local JNIEnv* env = [] {
            JNIEnv* env;
            javaVM()->AttachCurrentThread(&env, nullptr);
            javaVM()->GetEnv((void**)&env, JNI_VERSION_1_2);
            assert(env);
            return env;
        }();

        return env;
    }
}
