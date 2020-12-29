//
// Created by Алексей on 25.07.2018.
//

#pragma once

#include <AUI/Common/AString.h>
#include <glm/glm.hpp>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AVector.h>
#include <AUI/Views.h>

class AString;

namespace GL {
	class API_AUI_VIEWS Shader {
	private:
		uint32_t mProgram;
		uint32_t mVertex = 0;
		uint32_t mFragment = 0;
		uint32_t load(const AString&, uint32_t type);
		mutable AMap<AString, int32_t> mUniforms;
		int32_t getLocation(const AString& name) const;
		
	public:
		Shader();
		void load(const AString& vertex, const AString& fragment, const AVector<AString>& attribs = {}, const AString& version = {});
		void compile();
		void bindAttribute(uint32_t index, const AString& name);
		void use() const;
		~Shader();
		Shader(const Shader&) = delete;

		void set(const AString& uniform, int value) const;
		void set(const AString& uniform, float value) const;
		void set(const AString& uniform, double value) const;
		void set(const AString& uniform, glm::mat4 value) const;
		void set(const AString& uniform, glm::dmat4 value) const;
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
