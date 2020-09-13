#ifdef __ANDROID__

#include <jni.h>
#include "AWindow.h"

extern "C"
JNIEXPORT void JNICALL
Java_ru_alex2772_aui_MyGLRenderer_handleRedraw(JNIEnv *env, jclass clazz) {
    if (auto w = AWindow::current())
        w->loop();
}

extern "C"
JNIEXPORT void JNICALL
Java_ru_alex2772_aui_MyGLRenderer_handleResize(JNIEnv *env, jclass clazz, jint width, jint height) {
    if (auto w = AWindow::current())
        w->setSize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_ru_alex2772_aui_MyGLSurfaceView_handleMouseButtonDown(JNIEnv *env, jclass clazz, jint x,
                                                           jint y) {
    if (auto w = AWindow::current())
        w->onMousePressed({x, y}, AInput::LButton);
}
extern "C"
JNIEXPORT void JNICALL
Java_ru_alex2772_aui_MyGLSurfaceView_handleMouseButtonUp(JNIEnv *env, jclass clazz, jint x,
                                                           jint y) {
    if (auto w = AWindow::current())
        w->onMouseReleased({x, y}, AInput::LButton);
}


#endif