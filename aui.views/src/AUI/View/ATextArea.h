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

//
// Created by alex2 on 5/22/2021.
//


#pragma once


#include "AViewContainer.h"
#include "AAbstractTextField.h"
#include "AScrollbar.h"
#include "ATextBase.h"

/**
 * @brief Multiline text input area.
 * @ingroup useful_views
 * @details
 * Word breaking text area.
 *
 * In contrast to ATextField, ATextArea is not scrollable. It is often preferable to use a dedicated scroll area to make
 * the entire application pages scrollable with other content rather than using nested scrolls just for text area.
 *
 * If you want to make AScrollArea scrollable, it can be places inside a AScrollArea:
 * @code{cpp}
 * AScrollArea::Builder().withContents(_new<AScrollArea>())
 * @endcode
 */
class API_AUI_VIEWS ATextArea: public AAbstractTypeableView<ATextBase<ATextArea, AWordWrappingEngine<std::list<_unique<aui::detail::TextBaseEntry>>>>>, public IStringable {
public:
    friend class ATextBase<ATextArea, AWordWrappingEngine<std::list<_unique<aui::detail::TextBaseEntry>>>>;
    friend class UITextArea; // for testing

    using Iterator = Entries::iterator;

    ATextArea();
    ATextArea(const AString& text);
    ~ATextArea() override;

    bool capturesFocus() override;

    AString toString() const override;
    const AString& text() const override;
    unsigned int cursorIndexByPos(glm::ivec2 pos) override;
    glm::ivec2 getPosByIndex(size_t index) override;
    void setText(const AString& t) override;

    void render(ARenderContext context) override;

    void onCharEntered(char16_t c) override;

private:
    void onCursorIndexChanged() override;

protected:
    void typeableErase(size_t begin, size_t end) override;
    bool typeableInsert(size_t at, const AString& toInsert) override;
    bool typeableInsert(size_t at, char16_t toInsert) override;
    size_t typeableFind(char16_t c, size_t startPos) override;
    size_t typeableReverseFind(char16_t c, size_t startPos) override;
    size_t length() const override;

    auto wordEntries() const;
    auto charEntries() const;

private:
    mutable AOptional<AString> mCompiledText;
    glm::ivec2 mCursorPosition{0, 0};

    auto& entities() {
        return mEngine.entries();
    }

    auto& entities() const {
        return mEngine.entries();
    }

    struct EntityQueryResult {
        Iterator iterator;
        size_t relativeIndex;
    };

    EntityQueryResult getLeftEntity(size_t indexRelativeToFrom, EntityQueryResult from);
    EntityQueryResult getLeftEntity(size_t index);
};


