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

//
// Created by alex2772 on 25.07.2018.
//

#include "Program.h"
#include "State.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <AUI/Logging/ALogger.h>

gl::Program::Program() { mProgram = glCreateProgram(); }

void gl::Program::load(
    const AString& vertex, const AString& fragment, const AVector<AString>& attribs, const AString& version) {
#if AUI_PLATFORM_ANDROID
    AString prefix;

    if (version.empty()) {
    } else {
        prefix = "#version " + version + "\n";
    }

    prefix +=
        "precision mediump float;"
        "precision mediump int;";
#elif AUI_PLATFORM_APPLE
    AString prefix;
#else
    AString prefix;
    if (version.empty()) {
        prefix = "#version 120\n";
    } else {
        prefix = "#version " + version + "\n";
    }
#endif
    mVertex = load(prefix + vertex, GL_VERTEX_SHADER, false);
    mFragment = load(prefix + fragment, GL_FRAGMENT_SHADER, false);

    unsigned index = 0;
    for (auto& s : attribs) bindAttribute(index++, s);

    compile();
}

void gl::Program::loadVertexShader(const AString& vertex, bool raw) { mVertex = load(vertex, GL_VERTEX_SHADER, raw); }
void gl::Program::loadFragmentShader(const AString& fragment, bool raw) { mFragment = load(fragment, GL_FRAGMENT_SHADER, raw); }

void gl::Program::loadRaw(const AString& vertex, const AString& fragment) {
    loadVertexShader(vertex, true);
    loadFragmentShader(fragment, true);
}

gl::Program::~Program() {
    if (mProgram == 0) {
        return;
    }
    glDetachShader(mProgram, mVertex);
    glDetachShader(mProgram, mFragment);
    glDeleteShader(mVertex);
    glDeleteShader(mFragment);
    glDeleteProgram(mProgram);
}

uint32_t gl::Program::load(std::string code, uint32_t type, bool raw) {
    assert(!code.empty());

    if (!raw) {
#if AUI_PLATFORM_APPLE
        code = "#version 100\n"
            "precision mediump float;\n"
            "precision mediump int;\n" +
                code;
#endif
    }

    uint32_t shader = glCreateShader(type);
    AUI_ASSERT(!code.empty());
    const char* c = code.c_str();
    glShaderSource(shader, 1, &c, nullptr);
    glCompileShader(shader);
    int st;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &st);

    {
        char buf[8192];
        GLsizei len;
        glGetShaderInfoLog(shader, sizeof(buf), &len, buf);
        if (len) {
            ALogger::warn("OpenGL") << "Shader compile message: " << buf;
        }
    }
    if (!st) {
        throw AException("Failed to compile shader:\n" + code);
    }

    return shader;
}

void gl::Program::compile() {
    glAttachShader(mProgram, mVertex);
    glAttachShader(mProgram, mFragment);
    glLinkProgram(mProgram);
}

void gl::Program::use() const {
    gl::State::useProgram(mProgram);
    currentShader() = const_cast<gl::Program*>(this);
}

void gl::Program::bindAttribute(uint32_t index, const AString& name) {
    if (name.empty())
        return;
    glBindAttribLocation(mProgram, index, name.toStdString().c_str());
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::mat4 value) const {
    if (sameAsCache(uniform, value))
        return;
    auto loc = getLocation(uniform);
    if (loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_FALSE, &(value[0][0]));
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::mat3 value) const {
    if (sameAsCache(uniform, value))
        return;
    auto loc = getLocation(uniform);
    if (loc >= 0)
        glUniformMatrix3fv(loc, 1, GL_FALSE, &(value[0][0]));
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::dmat4 value) const {
    if (sameAsCache(uniform, value))
        return;
    auto loc = getLocation(uniform);
    if (loc >= 0) {
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS
        glm::mat4 fvalue = value;
        glUniformMatrix4fv(loc, 1, GL_FALSE, &(fvalue[0][0]));
#else
        glUniformMatrix4dv(loc, 1, GL_FALSE, &(value[0][0]));
#endif
    }
}

void gl::Program::set(const gl::Program::Uniform& uniform, float value) const {
    if (sameAsCache(uniform, value))
        return;
    auto loc = getLocation(uniform);
    if (loc >= 0)
        glUniform1f(loc, value);
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::vec2 value) const {
    if (sameAsCache(uniform, value))
        return;
    auto loc = getLocation(uniform);
    if (loc >= 0)
        glUniform2f(loc, value.x, value.y);
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::vec3 value) const {
    if (sameAsCache(uniform, value))
        return;
    auto loc = getLocation(uniform);
    if (loc >= 0)
        glUniform3f(loc, value.x, value.y, value.z);
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::vec4 value) const {
    if (sameAsCache(uniform, value))
        return;
    auto loc = getLocation(uniform);
    if (loc >= 0)
        glUniform4f(loc, value.x, value.y, value.z, value.a);
}

void gl::Program::set(const gl::Program::Uniform& uniform, int value) const {
    if (sameAsCache(uniform, value))
        return;
    auto loc = getLocation(uniform);
    if (loc >= 0)
        glUniform1i(loc, value);
}

void gl::Program::set(const gl::Program::Uniform& uniform, double value) const {
    if (sameAsCache(uniform, value))
        return;
    auto loc = getLocation(uniform);
    if (loc >= 0) {
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS
        glUniform1f(loc, value);
#else
        glUniform1d(loc, value);
#endif
    }
}

int32_t gl::Program::getLocation(const gl::Program::Uniform& uniform) const {
    auto& cache = mUniforms[uniform.getId()];
    auto& location = cache.id;
    if (location == UniformState::UNINITIALIZED) {
        location = glGetUniformLocation(mProgram, uniform.getUniformName());
        // assert(location != -1);
    }
    return location;
}

void gl::Program::setArray(const gl::Program::Uniform& uniform, AArrayView<float> value) const {
    auto loc = getLocation(uniform);
    if (loc >= 0) {
        glUniform1fv(loc, value.size(), value.data());
    }
}
unsigned gl::Program::Uniform::next() {
    static unsigned id = 0;
    return id++;
}
