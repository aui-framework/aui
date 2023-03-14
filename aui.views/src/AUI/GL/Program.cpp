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

//
// Created by alex2772 on 25.07.2018.
//

#include "Program.h"
#include "State.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <AUI/Logging/ALogger.h>


gl::Program::Program() {
    for (int32_t& uniform : mUniforms)
    {
        uniform = UniformState::UNINITIALIZED;
    }
	mProgram = glCreateProgram();
}

void gl::Program::load(const AString& vertex, const AString& fragment, const AVector<AString>& attribs, const AString& version)
{
#if AUI_PLATFORM_ANDROID
	AString prefix = "precision mediump float;"
					 "precision mediump int;"
					 ;
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
	for (auto& s : attribs)
		bindAttribute(index++, s);
	
	compile();
}
void gl::Program::loadRaw(const AString& vertex, const AString& fragment)
{
	mVertex = load(vertex, GL_VERTEX_SHADER, true);
	mFragment = load(fragment, GL_FRAGMENT_SHADER, true);
}

gl::Program::~Program() {
	glDetachShader(mProgram, mVertex);
	glDetachShader(mProgram, mFragment);
	glDeleteShader(mVertex);
	glDeleteShader(mFragment);
	glDeleteProgram(mProgram);
}


uint32_t gl::Program::load(const AString& data, uint32_t type, bool raw) {
	std::string code = data.toStdString();
	assert(!code.empty());

    if (!raw) {
#if AUI_PLATFORM_APPLE
        if (type == GL_VERTEX_SHADER) {

            code =
    "#define attribute in\n"
    "#define varying out\n"
    + code;
        } else {
            code =
    "#define varying in\n"
    "out vec4 resultColor;\n"
    "#define gl_FragColor resultColor\n"
    "#define texture2D texture\n"
    + code;
        }
#endif

#if AUI_PLATFORM_IOS
        code = "#version 300 es\n"
                "precision mediump float;\n"
                "precision mediump int;\n"
                         + code;
#elif AUI_PLATFORM_MACOS
        code = "#version 150\n" + code;
#endif
    }

    uint32_t shader = glCreateShader(type);
    assert(!code.empty());
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
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniformMatrix4fv(loc, 1, GL_FALSE, &(value[0][0]));
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::dmat4 value) const {
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
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform1f(loc, value);
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::vec2 value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform2f(loc, value.x, value.y);
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::vec3 value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform3f(loc, value.x, value.y, value.z);
}

void gl::Program::set(const gl::Program::Uniform& uniform, glm::vec4 value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform4f(loc, value.x, value.y, value.z, value.a);
}

void gl::Program::set(const gl::Program::Uniform& uniform, int value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform1i(loc, value);
}

void gl::Program::set(const gl::Program::Uniform& uniform, double value) const {
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
    int32_t& location = mUniforms[uniform.getId()];
    if (location == UniformState::UNINITIALIZED) {
        location = glGetUniformLocation(mProgram, uniform.getUniformName());
        //assert(location != -1);
    }
    return location;
}

/*
void gl::Shader::setArray(const gl::Shader::Uniform& uniform, const Vector<float>& value) {
	for (size_t i = 0; i < value.size(); ++i) {
		set(uniform + "[" + AString::number(i) + "]", value[i]);
	}
}*/
unsigned gl::Program::Uniform::next() {
    static unsigned id = 0;
    return id++;
}
