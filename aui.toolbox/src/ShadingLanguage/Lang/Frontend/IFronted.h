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