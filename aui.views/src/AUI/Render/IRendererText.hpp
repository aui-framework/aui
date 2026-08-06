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

#pragma once

#include <AUI/Common/SharedPtr.h>
#include <AUI/Common/AStringView.h>
#include "ATextLayoutHelper.h"

class ACanvas;

namespace aui {
class IPrerenderedString {
public:
    virtual void draw(ACanvas& canvas) = 0;
    virtual ~IPrerenderedString() = default;
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
};

class IMultiStringCanvas {
private:
    AOptional<ATextLayoutHelper::Symbols> mSymbols;

protected:

    /**
     * @brief Notifies IMultiStringCanvas than a symbol was added used to construct a ATextLayoutHelper.
     * @details
     * @param symbol symbol data to add
     * @details
     * This method should be called by the implementation of IMultiStringCanvas.
     *
     * At the end of line, implementation must add extra symbol to mark last position.
     */
    void notifySymbolAdded(const ATextLayoutHelper::Boundary& symbol) noexcept {
        if (mSymbols) mSymbols->last().push_back(symbol);
    }

public:
    virtual ~IMultiStringCanvas() = default;

    /**
     * @brief Notifies IMultiStringCanvas that getTextLayoutHelper() will be used.
     */
    void enableCachingForTextLayoutHelper() noexcept {
        mSymbols = ATextLayoutHelper::Symbols{};
        nextLine();
    }

    /**
     * @brief When caching for text layout helper is enabled, a new line added.
     */
    void nextLine() noexcept {
        if (mSymbols) mSymbols->push_back({});
    }

    /**
     * @brief Bakes a UTF-8 string with some position.
     * @param position position
     * @param text text
     */
    virtual void addString(const glm::ivec2& position, AStringView text) noexcept = 0;

    /**
     * @brief Bakes a UTF-32 string with some position.
     * @param position position
     * @param text text
     */
    virtual void addString(const glm::ivec2& position, std::u32string_view text) noexcept = 0;

    /**
     * @brief Bakes multi string canvas to IPrerenderedString which can be used for drawing text.
     * @return instance of <code>_<aui::IPrerenderedString></code> to drawElements with.
     * @details
     * Invalidates IMultiStringCanvas which speeds up some implementations of IMultiStringCanvas.
     */
    virtual _<IPrerenderedString> finalize() noexcept = 0;

    /**
     * @brief Returns text layout helper.
     * @return an instance of <code>IRenderer::ITextLayoutHelper</code> constructed from
     * <code>IMultiStringCanvas</code>'s cache to efficiently map cursor position to the string index.
     * @details
     * Call enableCachingForTextLayoutHelper before adding strings.
     *
     * Can be called only once.
     */
    ATextLayoutHelper getTextLayoutHelper() noexcept {
        AUI_ASSERTX(bool(mSymbols), "call enableCachingForTextLayoutHelper() before using getTextLayoutHelper");
        return ATextLayoutHelper(std::move(*mSymbols));
    }
};
}
