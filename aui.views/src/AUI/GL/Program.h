// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#pragma once

#include <AUI/Common/AString.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <optional>
#include <variant>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AVector.h>
#include <AUI/Views.h>

class AString;

namespace gl {
	class API_AUI_VIEWS Program {
	public:

        class API_AUI_VIEWS Uniform
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

		Program();
		void load(const AString& vertex, const AString& fragment, const AVector<AString>& attribs = {}, const AString& version = {});
		void loadRaw(const AString& vertex, const AString& fragment);
		void compile();
		void bindAttribute(uint32_t index, const AString& name);
		void use() const;
		~Program();
		Program(const Program&) = delete;

        uint32_t handle() const {
            return mProgram;
        }

        int32_t getLocation(const Uniform& uniform) const;

		void set(const gl::Program::Uniform& uniform, int value) const;
		void set(const gl::Program::Uniform& uniform, float value) const;
		void set(const gl::Program::Uniform& uniform, double value) const;
		void set(const gl::Program::Uniform& uniform, glm::mat4 value) const;
		void set(const gl::Program::Uniform& uniform, glm::mat3 value) const;
		void set(const gl::Program::Uniform& uniform, glm::dmat4 value) const;
		void set(const gl::Program::Uniform& uniform, glm::vec2 value) const;
		void set(const gl::Program::Uniform& uniform, glm::vec3 value) const;
		void set(const gl::Program::Uniform& uniform, glm::vec4 value) const;

		void setArray(const gl::Program::Uniform& uniform, const AVector<float>& value) const;

		static gl::Program*& currentShader()
		{
			static gl::Program* c;
			return c;
		}

	private:
		uint32_t mProgram;
		uint32_t mVertex = 0;
		uint32_t mFragment = 0;
		uint32_t load(const AString&, uint32_t type, bool raw);

        struct UniformCache {
            int32_t id = UniformState::UNINITIALIZED;
            using Value = std::variant<std::nullopt_t, int, float, double ,glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, glm::dmat4>;
            Value lastValue = std::nullopt;
        };
        mutable UniformCache mUniforms[64];

        class UniformState {
        public:
            enum Value {
                UNINITIALIZED = -2,
                DOES_NOT_EXIST = -1
            };
        };

        template<typename T>
        inline bool sameAsCache(const gl::Program::Uniform& uniform, T value) const {
            if (auto p = std::get_if<T>(&mUniforms[uniform.getId()].lastValue)) {
                return *p == value;
            }
            return false;
        }
    };
}
