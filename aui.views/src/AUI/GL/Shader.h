/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2772 on 25.07.2018.
//

#pragma once

#include <AUI/Common/AString.h>
#include <glm/glm.hpp>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AVector.h>
#include <AUI/Views.h>

class AString;

namespace gl {
	class API_AUI_VIEWS Shader {
	private:
		uint32_t mProgram;
		uint32_t mVertex = 0;
		uint32_t mFragment = 0;
		uint32_t load(const AString&, uint32_t type);
        mutable int32_t mUniforms[64];

        class UniformState {
        public:
            enum Value {
                UNINITIALIZED = -2,
                DOES_NOT_EXIST = -1
            };
        };
		
	public:

        class Uniform
        {
        private:
            const char* mUniformString;
            unsigned mId = -1;

            static unsigned next();

        public:
            explicit Uniform(const char* uniformString)
                    : mUniformString(uniformString), mId(next())
            {
            }


            const char* getUniformName() const
            {
                return mUniformString;
            }

            unsigned getId() const
            {
                return mId;
            }
        };

		Shader();
		void load(const AString& vertex, const AString& fragment, const AVector<AString>& attribs = {}, const AString& version = {});
		void compile();
		void bindAttribute(uint32_t index, const AString& name);
		void use() const;
		~Shader();
		Shader(const Shader&) = delete;

        int32_t getLocation(const Uniform& uniform) const;

		void set(const gl::Shader::Uniform& uniform, int value) const;
		void set(const gl::Shader::Uniform& uniform, float value) const;
		void set(const gl::Shader::Uniform& uniform, double value) const;
		void set(const gl::Shader::Uniform& uniform, glm::mat4 value) const;
		void set(const gl::Shader::Uniform& uniform, glm::dmat4 value) const;
		void set(const gl::Shader::Uniform& uniform, glm::vec2 value) const;
		void set(const gl::Shader::Uniform& uniform, glm::vec3 value) const;
		void set(const gl::Shader::Uniform& uniform, glm::vec4 value) const;

		void setArray(const gl::Shader::Uniform& uniform, const AVector<float>& value) const;

		static gl::Shader*& currentShader()
		{
			static gl::Shader* c;
			return c;
		}
	};
}
