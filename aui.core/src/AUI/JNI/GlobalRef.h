/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
        bool mLocal = false;

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
            if (!mObject) return;
            if (mLocal) {
                aui::jni::env()->DeleteLocalRef(mObject);
            } else {
                aui::jni::env()->DeleteGlobalRef(mObject);
            }
        }

        bool operator!() const {
            return !mObject;
        }

        operator bool() const {
            return mObject;
        }


        [[nodiscard]]
        jobject asObject() const noexcept {
            return mObject;
        }

        [[nodiscard]]
        jclass asClass() const noexcept {
            return (jclass)mObject;
        }

        static void assignLocalRef(GlobalRef& target, jobject value) {
            AUI_ASSERT(target.mObject == nullptr);
            target.mObject = value;
            target.mLocal = true;
        }
    };
}
