#pragma once


#include <AUI/View/AViewContainer.h>
#include <AUI/Model/IListModel.h>
#include <AUI/Model/IMutableListModel.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AButton.h>

class ABasicListEditor: public AViewContainer {
private:

public:

    class Builder {
    private:
        _<IListModel<AString>> mModel;
        AString mTitle;
        std::function<void()> mNewCallback;
        std::function<void(const AModelIndex&)> mModifyCallback;

    public:
        Builder(const _<IListModel<AString>>& model): mModel(model) {}


        Builder& withNewButton(const std::function<void()>& onNew) {
            mNewCallback = onNew;
            return *this;
        }
        Builder& withModifyButton(const std::function<void(const AModelIndex&)>& onModify) {
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
                callback(list->getSelectionModel().one());
            }) : nullptr;

            _<AButton> removeButton;
            if (auto model = _cast<IMutableListModel<AString>>(mModel)) {
                removeButton = _new<AButton>("Remove").connect(&AView::clicked, c, [list, model] {
                    model->removeItems(list->getSelectionModel());
                });
            }

            if (mModel->listSize() != 0) {
                list->selectItem(0);
            } else {
                nullsafe(removeButton)->disable();
                nullsafe(modifyButton)->disable();
            }
            auto updateEnabledState = [list, modifyButton, removeButton]() {
                auto& s = list->getSelectionModel().getIndices();
                nullsafe(modifyButton)->setDisabled(s.empty());
                nullsafe(removeButton)->setDisabled(s.empty());
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


