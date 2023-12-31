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
Java_com_github_aui_android_AuiView_handleInit(JNIEnv *env, jclass clazz, jstring internalStoragePathR) {
    jboolean isCopy;
    auto internalStoragePath = env->GetStringUTFChars(internalStoragePathR, &isCopy);
    chdir(internalStoragePath);
    env->ReleaseStringUTFChars(internalStoragePathR, internalStoragePath);

#ifdef AUI_CATCH_UNHANDLED
    aui_init_signal_handler();
#endif
    _gEntry({});
}
