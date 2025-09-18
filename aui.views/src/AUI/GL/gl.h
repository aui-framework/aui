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
#ifdef __cplusplus
extern "C" {
#endif
#if AUI_PLATFORM_ANDROID
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#elif AUI_PLATFORM_MACOS
#include <OpenGL/OpenGL.h>
#elif AUI_PLATFORM_IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif
#ifdef __cplusplus
}
#endif

#if AUI_PLATFORM_MACOS
#include <GL/glew.h>
#endif
