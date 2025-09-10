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
#include <AUI/Util/AArrayView.h>

class AString;

namespace gl {
enum Precision : uint8_t {
    mediump,
    highp,
};
struct GLSLOptions {
    Precision floatp = mediump;
    Precision intp = mediump;
};

class API_AUI_VIEWS Program: public aui::noncopyable {
public:
    class API_AUI_VIEWS Uniform {
    private:
        const char* mUniformString;
        unsigned mId = -1;

        static unsigned next();

    public:
        explicit Uniform(const char* uniformString) : mUniformString(uniformString), mId(next()) {}

        const char* getUniformName() const { return mUniformString; }

        unsigned getId() const { return mId; }
    };

    Program();
    explicit Program(Program&& program) noexcept {
        operator=(std::move(program));
    }
    void
    load(const AString& vertex, const AString& fragment, const AVector<AString>& attribs = {},
         GLSLOptions options = {});
    void loadVertexShader(const AString& vertex, GLSLOptions options = {});
    void loadFragmentShader(const AString& fragment, GLSLOptions options = {});
    void loadBoth(const AString& vertex, const AString& fragment, GLSLOptions options = {}) {
        loadVertexShader(vertex, options);
        loadFragmentShader(fragment, options);
    }
    void compile();
    void bindAttribute(uint32_t index, const AString& name);
    void use() const;
    ~Program();
    Program(const Program&) = delete;

    uint32_t handle() const { return mProgram; }

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

    void setArray(const gl::Program::Uniform& uniform, AArrayView<float> value) const;

    static gl::Program*& currentShader() {
        static gl::Program* c;
        return c;
    }

    Program& operator=(Program&& program) noexcept {
        if (this == &program) {
            return *this;
        }
        mProgram = std::exchange(program.mProgram, 0);
        mUniforms = std::exchange(program.mUniforms, {});
        mVertex = std::exchange(program.mVertex, 0);
        mFragment = std::exchange(program.mFragment, 0);

        return *this;
    }

private:
    uint32_t mProgram;
    uint32_t mVertex = 0;
    uint32_t mFragment = 0;

    class UniformState {
    public:
        enum Value { UNINITIALIZED = -2, DOES_NOT_EXIST = -1 };
    };

    struct UniformCache {
        int32_t id = UniformState::UNINITIALIZED;
        using Value = std::variant<
            std::nullopt_t, int, float, double, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, glm::dmat4>;
        Value lastValue = std::nullopt;
    };
    mutable std::array<UniformCache, 64> mUniforms;

    template <typename T>
    inline bool sameAsCache(const gl::Program::Uniform& uniform, T value) const {
        if (auto p = std::get_if<T>(&mUniforms[uniform.getId()].lastValue)) {
            return *p == value;
        }
        return false;
    }
    uint32_t load(std::string code, uint32_t type, GLSLOptions options = {});
};
}   // namespace gl
