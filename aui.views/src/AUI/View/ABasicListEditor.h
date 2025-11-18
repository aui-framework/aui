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


#include <AUI/View/AViewContainer.h>
#include <AUI/Model/IListModel.h>
#include <AUI/Model/IMutableListModel.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AButton.h>


/**
 * @brief Helper UI to edit list models.
 */
class ABasicListEditor: public AViewContainerBase {
private:

public:

    class Builder {
    private:
        _<IListModel<AString>> mModel;
        AString mTitle;
        std::function<void()> mNewCallback;
        std::function<void(const AListModelIndex&)> mModifyCallback;

    public:
        Builder(const _<IListModel<AString>>& model): mModel(model) {}


        Builder& withNewButton(const std::function<void()>& onNew) {
            mNewCallback = onNew;
            return *this;
        }
        Builder& withModifyButton(const std::function<void(const AListModelIndex&)>& onModify) {
            mModifyCallback = onModify;
            return *this;
        }

        _<AWindow> buildWindow(const AString& title, AWindow* parent = nullptr) {
            auto window = AWindow::wrapViewToWindow(buildContainer(),
                                                    title,
                                                    300_dp,
                                                    400_dp,
                                                    parent,
                                                    WindowStyle::MODAL |
                                                    WindowStyle::NO_RESIZE |
                                                    WindowStyle::NO_MINIMIZE_MAXIMIZE);
            return window;
        }

        _<ABasicListEditor> buildContainer() {
            using namespace declarative;
            auto c = _new<ABasicListEditor>();
            auto list = _new<AListView>(mModel);
            list AUI_OVERRIDE_STYLE { ass::MinSize { 200_dp, {} } };

            auto modifyButton = mModifyCallback ? _new<AButton>("Modify...").connect(&AView::clicked, c, [callback = mModifyCallback, list] {
                callback(list->getSelectionModel().first());
            }) : nullptr;

            _<AButton> removeButton;
            if (auto model = _cast<IRemovableListModel<AString>>(mModel)) {
                removeButton = _new<AButton>("Remove").connect(&AView::clicked, c, [list, model] {
                    model->removeItems(list->getSelectionModel());
                });
            }

            if (mModel->listSize() != 0) {
                list->selectItem(0);
            } else {
                AUI_NULLSAFE(removeButton)->disable();
                AUI_NULLSAFE(modifyButton)->disable();
            }
            auto updateEnabledState = [list, modifyButton, removeButton]() {
                auto& s = list->getSelectionModel().getIndices();
                AUI_NULLSAFE(modifyButton)->setDisabled(s.empty());
                AUI_NULLSAFE(removeButton)->setDisabled(s.empty());
            };
            connect(list->selectionChanged, c, updateEnabledState);
            connect(mModel->dataRemoved, c, updateEnabledState);
            connect(mModel->dataInserted, c, updateEnabledState);

            c->setContents(Horizontal {
                list,
                Vertical {
                    mNewCallback ? _new<AButton>("New...").connect(&AView::clicked, c, [callback = mNewCallback] {
                        callback();
                    }) : nullptr,
                    modifyButton,
                    removeButton
                },
            });

            return c;
        }
    };
};


