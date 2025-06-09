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


#include <AUI/Logging/ALogger.h>
#include "OSAndroid.h"
#include <AUI/Platform/Entry.h>
#include <unistd.h>
#include <AUI/Common/AByteBuffer.h>

extern void aui_init_signal_handler();

int(*_gEntry)(const AStringVector&);

AUI_EXPORT int aui_main(JavaVM* vm, int(*aui_entry)(const AStringVector&)) {
    aui::jni::setJavaVM(vm);
    _gEntry = aui_entry;
    return 0;
}

const ACommandLineArgs& aui::args() noexcept {
    static ACommandLineArgs args;
    return args; 
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AUIView_handleInit__Ljava_lang_String_2(JNIEnv *env, jclass clazz, jstring internalStoragePathR) {
    jboolean isCopy;
    auto internalStoragePath = env->GetStringUTFChars(internalStoragePathR, &isCopy);
    chdir(internalStoragePath);
    env->ReleaseStringUTFChars(internalStoragePathR, internalStoragePath);

#ifdef AUI_CATCH_UNHANDLED
    aui_init_signal_handler();
#endif
    {
        [[maybe_unused]] auto mainThread = AThread::main();
    }
    _gEntry({});
}
