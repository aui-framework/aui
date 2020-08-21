//
// Created by Алексей on 25.07.2018.
//

#pragma once

#include "gl.h"
#include <AUI/Common/AString.h>
#include <glm/glm.hpp>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AVector.h>
#include <AUI/Views.h>

class AString;

namespace GL {
	class API_AUI_VIEWS Shader {
	private:
		GLuint mProgram;
		GLuint mVertex = 0;
		GLuint mFragment = 0;
		GLuint load(const AString&, GLenum type);
		mutable AMap<AString, GLint> mUniforms;
		GLint getLocation(const AString& name) const;
		
	public:
		Shader();
		void load(const AString& vertex, const AString& fragment, const AVector<AString>& attribs = {});
		void compile();
		void bindAttribute(GLuint index, const AString& name);
		void use() const;
		~Shader();
		Shader(const Shader&) = delete;

		void set(const AString& uniform, int value) const;
		void set(const AString& uniform, float value) const;
		void set(const AString& uniform, glm::mat4 value) const;
		void set(const AString& uniform, glm::vec2 value) const;
		void set(const AString& uniform, glm::vec3 value) const;
		void set(const AString& uniform, glm::vec4 value) const;

		void setArray(const AString& uniform, const AVector<float>& value) const;

		static GL::Shader*& currentShader()
		{
			static GL::Shader* c;
			return c;
		}
	};
}
