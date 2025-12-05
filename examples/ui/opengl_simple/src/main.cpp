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

/// [OpenGL_example]
#include "AUI/GL/State.h"
#include "AUI/Platform/ARenderingContextOptions.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>

using namespace ass;
using namespace declarative;

namespace {

struct State {
    AProperty<float> fps = 0;
};

class MyRenderer : public AView {
public:
    MyRenderer(_<State> state) : mState(std::move(state)) {
        // Simple passthrough ES 2.0 shaders
        const char* vsSrc =
            "attribute vec2 aPos;\n"
            "void main(){\n"
            "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
            "}\n";
        const char* fsSrc =
            "precision mediump float;\n"
            "void main(){\n"
            "  gl_FragColor = vec4(1.0, 0.4, 0.2, 1.0);\n"
            "}\n";

        // Compile vertex shader
        mShaderVertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(mShaderVertex, 1, &vsSrc, nullptr);
        glCompileShader(mShaderVertex);

        // Compile fragment shader
        mShaderFragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(mShaderFragment, 1, &fsSrc, nullptr);
        glCompileShader(mShaderFragment);

        // Link program
        mProgram = glCreateProgram();
        glAttachShader(mProgram, mShaderVertex);
        glAttachShader(mProgram, mShaderFragment);
        glBindAttribLocation(mProgram, 0, "aPos");
        glLinkProgram(mProgram);

        // Triangle vertices (clip space)
        const GLfloat verts[] = {
            -0.6f, -0.6f, 0.0f, 0.6f, 0.6f, -0.6f,
        };

        // Create VBO
        glGenBuffers(1, &mVertexBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    ~MyRenderer() override {
        if (mVertexBufferObject) {
            glDeleteBuffers(1, &mVertexBufferObject);
            mVertexBufferObject = 0;
        }
        if (mProgram) {
            if (mShaderVertex)
                glDetachShader(mProgram, mShaderVertex);
            if (mShaderFragment)
                glDetachShader(mProgram, mShaderFragment);
            glDeleteProgram(mProgram);
            mProgram = 0;
        }
        if (mShaderVertex) {
            glDeleteShader(mShaderVertex);
            mShaderVertex = 0;
        }
        if (mShaderFragment) {
            glDeleteShader(mShaderFragment);
            mShaderFragment = 0;
        }
    }

    void cleanupAUIGLState() {
        gl::State::activeTexture(0);
        gl::State::bindTexture(GL_TEXTURE_2D, 0);
        gl::State::bindVertexArray(0);
        gl::State::useProgram(0);
    }

    void render(ARenderContext ctx) override {
        AView::render(ctx);
        cleanupAUIGLState();

        // Draw triangle
        glUseProgram(mProgram);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObject);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void*) 0);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        cleanupAUIGLState();

        // calculate fps
        auto now = std::chrono::high_resolution_clock::now();
        mState->fps = 1'000'000.f / std::chrono::duration_cast<std::chrono::microseconds>(now - mLastFrame).count();
        mLastFrame = now;

        // request redraw if you'd like to show animations
        redraw();
    }

private:
    _<State> mState;
    std::chrono::high_resolution_clock::time_point mLastFrame {};
    GLuint mProgram {};
    GLuint mShaderVertex {};
    GLuint mShaderFragment {};
    GLuint mVertexBufferObject {};
};
}   // namespace

AUI_ENTRY {
    // ask aui to provide opengl context for us. (no fallback to software rendering)
    ARenderingContextOptions::set({
        .initializationOrder = {
            ARenderingContextOptions::OpenGL{},
        },
        .flags = ARenderContextFlags::NO_SMOOTH | ARenderContextFlags::NO_VSYNC,
    });

    auto window = _new<AWindow>("OpenGL Demo", 600_dp, 300_dp);
    auto state = _new<State>();
    window->setContents(
        Stacked {
          _new<MyRenderer>(state) AUI_OVERRIDE_STYLE { Expanding() },
          Horizontal::Expanding {
            Vertical {
              Vertical {
                Label { AUI_REACT(fmt::format("FPS: {:.1f}", *state->fps)) },
              } AUI_OVERRIDE_STYLE {
                    Padding(16_dp),
                    BackgroundSolid { AColor::WHITE.transparentize(0.5f) },
                    MinSize { 150_dp, {} },
                  },
            },
          },
        } AUI_OVERRIDE_STYLE { Padding(0) });
    window->show();
    return 0;
}
/// [OpenGL_example]
