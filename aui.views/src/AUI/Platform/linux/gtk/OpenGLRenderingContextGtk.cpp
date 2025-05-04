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

//
// Created by alex2772 on 5/2/25.
//

#include "OpenGLRenderingContextGtk.h"
#include "AUI/Platform/linux/IPlatformAbstraction.h"

OpenGLRenderingContextGtk::Texture::~Texture() {
    if (gl_texture) {
        gdk_gl_texture_release(GDK_GL_TEXTURE(gl_texture));
    }
    auto id = gdk_gl_texture_builder_get_id(builder);
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
    g_clear_object(&builder);
    if (gl_texture == nullptr && dmabuf_texture == nullptr) {
        g_free(gl_texture);
    }
}

void OpenGLRenderingContextGtk::init(const IRenderingContext::Init& init) {
}

void OpenGLRenderingContextGtk::destroyNativeWindow(AWindowBase& window) {
}


void OpenGLRenderingContextGtk::gtkRealize(GtkWidget* widget) {
    realCreateContext(widget);
    mNeedsResize = true;

    auto acquired = contextScope();
    if (!glewExperimental) {
        glewExperimental = true;
        switch (auto s = glewInit()) {
            default:
                glewExperimental = false;
                throw AException("glewInit failed");
                break;

            case GLEW_ERROR_NO_GLX_DISPLAY:
                // we can safely ignore that; see https://github.com/nigels-com/glew/issues/172
                [[fallthrough]];
            case GLEW_OK:
                break;
        }
        ALogger::info("OpenGL context is ready");
    }
    mRenderer = ourRenderer();
}

void OpenGLRenderingContextGtk::gtkSnapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
    if (mContext == nullptr) {
        return;
    }
    auto acquired = contextScope();
    attachBuffers(widget);
    if (mFramebufferForGtk == 0) {
        return;
    }
    glViewport(
        0, 0, gdk_gl_texture_builder_get_width(mTexture->builder),
        gdk_gl_texture_builder_get_height(mTexture->builder));

    IPlatformAbstraction::setCurrentWindow(&mWindow);

    gl::Framebuffer::DEFAULT_FB = mFramebufferForGtk;
    AWindow::getWindowManager().watchdog().runOperation([&] {
        if (auto w = dynamic_cast<AWindow*>(&mWindow)) {
            IPlatformAbstraction::redrawFlag(*w) = false;
        }
        mWindow.redraw();
    });

    auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    gdk_gl_texture_builder_set_sync(mTexture->builder, sync);
    mTexture->gl_texture = gdk_gl_texture_builder_build(
            mTexture->builder,
            [](gpointer data) {
                auto texture = reinterpret_cast<Texture*>(data);
                auto sync = (GLsync) gdk_gl_texture_builder_get_sync(texture->builder);
                if (sync != nullptr) {
                    glDeleteSync(sync);
                    gdk_gl_texture_builder_set_sync(texture->builder, nullptr);
                }
                texture->gl_texture = nullptr;
            },
            &*mTexture);

    gtk_snapshot_save(snapshot);
    {
        const auto scale =
            float(gdk_surface_get_scale(gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(widget)))));

        auto physicalWidth = gdk_gl_texture_builder_get_width(mTexture->builder);
        auto physicalHeight = gdk_gl_texture_builder_get_height(mTexture->builder);

        gtk_snapshot_scale(snapshot, 1.f / scale, 1.f / scale);
        auto rect = GRAPHENE_RECT_INIT(
            0.375f, 0.375f, // from opengl red book
            float(physicalWidth), float(physicalHeight));
        gtk_snapshot_append_scaled_texture(snapshot, mTexture->gl_texture, GSK_SCALING_FILTER_NEAREST, &rect);
    }
    gtk_snapshot_restore(snapshot);
}
void OpenGLRenderingContextGtk::endFramebuffer() {
    if (auto fb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
        // gtk can't flip the Y axis by itself, so we need to do it manually
        fb->bindForRead();
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl::Framebuffer::DEFAULT_FB);
        glBlitFramebuffer(
            0, 0,                                                 // src pos
            fb->supersampledSize().x, fb->supersampledSize().y,   // src size
            0, fb->size().y,                                      // dst pos
            fb->size().x, 0,                                      // dst size
            GL_COLOR_BUFFER_BIT,                                  // mask
            GL_LINEAR);                                           // filter
        gl::Framebuffer::unbind();
    }
}

void OpenGLRenderingContextGtk::gtkUnrealize(GtkWidget* widget) {
    deleteTextures();
    deleteBuffers();
}

void OpenGLRenderingContextGtk::realCreateContext(GtkWidget* widget) {
    GError* error = nullptr;
    mContext =
        gdk_surface_create_gl_context(gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(widget))), &error);
    if (error != nullptr) {
        g_warning("Failed to create GL context: %s", error->message);
        g_clear_object(&mContext);
        mContext = nullptr;
        g_clear_error(&error);
        return;
    }
    gdk_gl_context_set_allowed_apis(mContext, mAllowedApis);
    gdk_gl_context_set_required_version(mContext, 3, 0);
    gdk_gl_context_realize(mContext, &error);
    if (error != nullptr) {
        g_warning("Failed to create GL context: %s", error->message);
        g_clear_object(&mContext);
        mContext = nullptr;
        g_clear_error(&error);
    }
}

void OpenGLRenderingContextGtk::ensureTexture(GtkWidget* widget) {
    gtk_widget_realize(widget);
    if (mContext == nullptr) {
        return;
    }
    AUI_ASSERT(gdk_gl_context_get_current() == mContext);
    if (!mTexture) {
        mTexture.emplace();

        mTexture->builder = gdk_gl_texture_builder_new();
        gdk_gl_texture_builder_set_context(mTexture->builder, mContext);
        if (gdk_gl_context_get_api(mContext) == GDK_GL_API_GLES) {
            gdk_gl_texture_builder_set_format(mTexture->builder, GDK_MEMORY_R8G8B8A8_PREMULTIPLIED);
        } else {
            gdk_gl_texture_builder_set_format(mTexture->builder, GDK_MEMORY_B8G8R8A8_PREMULTIPLIED);
        }

        GLuint id;
        glGenTextures(1, &id);
        gdk_gl_texture_builder_set_id(mTexture->builder, id);
    }
    allocateTexture(widget);
}

void OpenGLRenderingContextGtk::allocateTexture(GtkWidget* widget) {
    if (mContext == nullptr) {
        return;
    }
    AUI_ASSERT(gdk_gl_context_get_current() == mContext);
    if (!mTexture) {
        return;
    }

    const auto scale = gdk_surface_get_scale(gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(widget))));
    //        const auto alignTo = std::lcm(int(std::floor(scale)), int(1.0 / std::fmod(scale, 1.0)));
    auto correctSize = [&](int i) {
      //            return int((i - (i % alignTo)) * scale);
      return int(std::ceil(i * scale));
    };

    int width = correctSize(gtk_widget_get_width(widget));
    int height = correctSize(gtk_widget_get_height(widget));

    assert(width > 0);
    assert(height > 0);

    if (gdk_gl_texture_builder_get_width(mTexture->builder) != width ||
        gdk_gl_texture_builder_get_height(mTexture->builder) != height) {
        auto texId = gdk_gl_texture_builder_get_id(mTexture->builder);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (gdk_gl_context_get_api(mContext) == GDK_GL_API_GLES)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

        gdk_gl_texture_builder_set_width(mTexture->builder, width);
        gdk_gl_texture_builder_set_height(mTexture->builder, height);

        mWindow.AViewContainer::setSize({width, height});
    }
}

void OpenGLRenderingContextGtk::beginResize(AWindowBase& window) {

}
void OpenGLRenderingContextGtk::endResize(AWindowBase& window) {

}

void OpenGLRenderingContextGtk::ensureBuffers(GtkWidget* widget) {
    gtk_widget_realize(widget);
    if (mContext == nullptr) {
        return;
    }
    if (mHaveBuffers) {
        return;
    }
    AUI_ASSERT(gdk_gl_context_get_current() == mContext);
    mHaveBuffers = true;
    glGenFramebuffers(1, &mFramebufferForGtk);

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferForGtk);
    if (!mTexture) {
        g_warning("No texture");
        deleteBuffers();
        return;
    }
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gdk_gl_texture_builder_get_id(mTexture->builder), 0);
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        g_warning("Framebuffer incomplete: %d", status);
        deleteBuffers();
    }
}

void OpenGLRenderingContextGtk::attachBuffers(GtkWidget* widget) {
    if (mContext == nullptr) {
        return;
    }
    AUI_ASSERT(gdk_gl_context_get_current() == mContext);

    if (!mTexture) {
        ensureTexture(widget);
    } else if (mNeedsResize) {
        allocateTexture(widget);
    }

    if (!mHaveBuffers) {
        ensureBuffers(widget);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferForGtk);
}

void OpenGLRenderingContextGtk::deleteBuffers() {
    if (mFramebufferForGtk == 0) {
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &mFramebufferForGtk);
    mFramebufferForGtk = 0;
}

void OpenGLRenderingContextGtk::deleteTextures() { mTexture.reset(); }

