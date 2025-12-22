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
#include "gtk_functions.h"

class OpenGLRenderingContextGtk: public OpenGLRenderingContext, public RenderingContextGtk {
public:
    OpenGLRenderingContextGtk(const ARenderingContextOptions::OpenGL& config, ASurface& window)
      : OpenGLRenderingContext(config), RenderingContextGtk(window) {}

    ~OpenGLRenderingContextGtk() override = default;
    void init(const Init& init);
    void destroyNativeWindow(ASurface& window) override;
    void gtkRealize(aui::gtk4_fake::GtkWidget* widget) override;
    void gtkSnapshot(aui::gtk4_fake::GtkWidget* widget, aui::gtk4_fake::GtkSnapshot* snapshot) override;
    void gtkUnrealize(aui::gtk4_fake::GtkWidget* widget) override;
    void beginResize(ASurface& window) override;
    void endResize(ASurface& window) override;

    auto contextScope() {
        using namespace aui::gtk4_fake;
        auto prev = gdk_gl_context_get_current();
        auto ctx = ourContext;
        if (ctx != nullptr) {
            AUI_ASSERT(prev != ctx);
            gdk_gl_context_make_current(ctx);
        }
        return aui::ptr::manage_unique(ctx, [prev](GdkGLContext*) {
            gdk_gl_context_clear_current();
            gdk_gl_context_make_current(prev);
        });
    }
    void gtkDoUnderContext(const std::function<void()>& callback) override; // this is soo bad

private:
    struct Texture {
        aui::gtk4_fake::GdkGLTextureBuilder* builder {};
        aui::gtk4_fake::GdkTexture* gl_texture {};
        aui::gtk4_fake::GdkTexture* dmabuf_texture {};

        ~Texture();
    };
    AOptional<Texture> mTexture;
    static aui::gtk4_fake::GdkGLContext* ourContext;
    GLuint mFramebufferForGtk = 0;
    bool mNeedsResize = false;
    bool mHaveBuffers = false;
    aui::gtk4_fake::GdkGLAPI mAllowedApis = static_cast<aui::gtk4_fake::GdkGLAPI>(aui::gtk4_fake::GDK_GL_API_GL | aui::gtk4_fake::GDK_GL_API_GLES);

    void realCreateContext(aui::gtk4_fake::GtkWidget* widget);
    void ensureTexture(aui::gtk4_fake::GtkWidget* widget);
    void allocateTexture(aui::gtk4_fake::GtkWidget* widget);
    void ensureBuffers(aui::gtk4_fake::GtkWidget* widget);
    void attachBuffers(aui::gtk4_fake::GtkWidget* widget);
    void deleteBuffers();
    void deleteTextures();

protected:
    void endFramebuffer() override;
};
