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

#pragma once

#include "ShadingLanguage/Lang/AST/AST.h"

enum class ShaderType {
    VERTEX,
    FRAGMENT, // or pixel
    IMPORTED, // those which are imported with 'import' keyword
};

class IFrontend {
public:
    virtual ~IFrontend() = default;

    /**
     * @brief Shader code for tests
     */
    virtual AString shaderCode() = 0;
    virtual void parseShader(const _<AST>& ast) = 0;

    virtual void writeCpp(const APath& destination) = 0;

    ShaderType shaderType() const {
        return mShaderType;
    }

    void setShaderType(ShaderType shaderType) {
        mShaderType = shaderType;
    }

    bool hasErrors() const {
        return mHasErrors;
    }

protected:
    void setHasError() {
        mHasErrors = true;
    }

private:
    ShaderType mShaderType;
    bool mHasErrors = false;
};