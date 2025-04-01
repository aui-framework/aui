/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
            AUI_ASSERT(env);
            return env;
        }();

        return env;
    }
}
