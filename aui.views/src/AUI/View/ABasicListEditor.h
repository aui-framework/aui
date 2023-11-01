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


#include <AUI/View/AViewContainer.h>
#include <AUI/Model/IListModel.h>
#include <AUI/Model/IMutableListModel.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AButton.h>


/**
 * @brief Helper UI to edit list models.
 */
class ABasicListEditor: public AViewContainer {
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
            auto c = _new<ABasicListEditor>();
            auto list = _new<AListView>(mModel);
            list with_style { ass::MinSize { 200_dp, {} } };

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


