//
// Created by Алексей on 25.07.2018.
//

#include "Shader.h"
#include "State.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>


GL::Shader::Shader() {
	mProgram = glCreateProgram();
}

void GL::Shader::load(const AString& vertex, const AString& fragment, const AVector<AString>& attribs)
{
	AString version = "#version 120\n";
	mVertex = load(version + vertex, GL_VERTEX_SHADER);
	mFragment = load(version + fragment, GL_FRAGMENT_SHADER);

	unsigned index = 0;
	for (auto& s : attribs)
		bindAttribute(index++, s);
	
	compile();
}

GL::Shader::~Shader() {
	glDetachShader(mProgram, mVertex);
	glDetachShader(mProgram, mFragment);
	glDeleteShader(mVertex);
	glDeleteShader(mFragment);
	glDeleteProgram(mProgram);
}


GLuint GL::Shader::load(const AString& data, GLenum type) {
	GLuint shader = glCreateShader(type);
	std::string code = data.toStdString();
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
			std::cout << buf << std::endl;
		}
	}

	return shader;
}

void GL::Shader::compile() {
	glAttachShader(mProgram, mVertex);
	glAttachShader(mProgram, mFragment);
	glLinkProgram(mProgram);
}

void GL::Shader::use() const {
	GL::State::useProgram(mProgram);
	currentShader() = const_cast<GL::Shader*>(this);
}

void GL::Shader::bindAttribute(GLuint index, const AString& name) {
	glBindAttribLocation(mProgram, index, name.toStdString().c_str());
}

GLint GL::Shader::getLocation(const AString& name) const {
	auto it = mUniforms.find(name);
	if (it == mUniforms.end()) {
		GLint loc = glGetUniformLocation(mProgram, name.toStdString().c_str());
		mUniforms[name] = loc;
		return loc;
	}
	return it->second;
}

void GL::Shader::set(const AString& uniform, glm::mat4 value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniformMatrix4fv(loc, 1, GL_FALSE, &(value[0][0]));
}

void GL::Shader::set(const AString& uniform, float value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform1f(loc, value);
}

void GL::Shader::set(const AString& uniform, glm::vec2 value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform2f(loc, value.x, value.y);
}

void GL::Shader::set(const AString& uniform, glm::vec3 value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform3f(loc, value.x, value.y, value.z);
}

void GL::Shader::set(const AString& uniform, glm::vec4 value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform4f(loc, value.x, value.y, value.z, value.a);
}

void GL::Shader::set(const AString& uniform, int value) const {
	auto loc = getLocation(uniform);
	if (loc >= 0)
		glUniform1i(loc, value);
}

/*
void GL::Shader::setArray(const AString& uniform, const Vector<float>& value) {
	for (size_t i = 0; i < value.size(); ++i) {
		set(uniform + "[" + AString::number(i) + "]", value[i]);
	}
}*/
