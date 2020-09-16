#pragma once
extern "C" {
#ifdef ANDROID
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif
}