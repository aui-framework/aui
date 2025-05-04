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

#include "RenderingContextGtk.h"
#include <AUI/Platform/OpenGLRenderingContext.h>

class OpenGLRenderingContextGtk: public OpenGLRenderingContext, public RenderingContextGtk {
public:
    OpenGLRenderingContextGtk(const ARenderingContextOptions::OpenGL& config, AWindowBase& window)
      : OpenGLRenderingContext(config), RenderingContextGtk(window) {}

    ~OpenGLRenderingContextGtk() override = default;
    void init(const Init& init) override;
    void destroyNativeWindow(AWindowBase& window) override;
    void gtkRealize(GtkWidget* widget) override;
    void gtkSnapshot(GtkWidget* widget, GtkSnapshot* snapshot) override;
    void gtkUnrealize(GtkWidget* widget) override;
    void beginResize(AWindowBase& window) override;
    void endResize(AWindowBase& window) override;

    auto contextScope() {
        auto prev = gdk_gl_context_get_current();
        auto ctx = ourContext;
        if (ctx != nullptr) {
            AUI_ASSERT(prev != ctx);
            gdk_gl_context_make_current(ctx);
        }
        return aui::ptr::make_unique_with_deleter(ctx, [prev](GdkGLContext*) {
            gdk_gl_context_clear_current();
            gdk_gl_context_make_current(prev);
        });
    }
    void gtkDoUnderContext(const std::function<void()>& callback) override; // this is soo bad

private:
    struct Texture {
        GdkGLTextureBuilder* builder {};
        GdkTexture* gl_texture {};
        GdkTexture* dmabuf_texture {};

        ~Texture();
    };
    AOptional<Texture> mTexture;
    static GdkGLContext* ourContext;
    GLuint mFramebufferForGtk = 0;
    bool mNeedsResize = false;
    bool mHaveBuffers = false;
    GdkGLAPI mAllowedApis = static_cast<GdkGLAPI>(GDK_GL_API_GL | GDK_GL_API_GLES);

    void realCreateContext(GtkWidget* widget);
    void ensureTexture(GtkWidget* widget);
    void allocateTexture(GtkWidget* widget);
    void ensureBuffers(GtkWidget* widget);
    void attachBuffers(GtkWidget* widget);
    void deleteBuffers();
    void deleteTextures();

protected:
    void endFramebuffer() override;
};
