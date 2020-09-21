#pragma once

#include <AUI/Model/AModelSelection.h>
#include "AViewContainer.h"
#include "AUI/Model/AModelIndex.h"
#include "AUI/Model/IListModel.h"

class AListItem;

class API_AUI_VIEWS AListView: public AViewContainer
{
private:
	_<IListModel<AString>> mModel;
	ASet<AModelIndex> mSelectionModel;
	
public:
    AListView(): AListView(nullptr) {}
	explicit AListView(const _<IListModel<AString>>& model);
    virtual ~AListView();

    void setModel(const _<IListModel<AString>>& model);

    int getContentMinimumHeight() override;
    int getContentFullHeight() {
        return getLayout()->getMinimumHeight() + 4;
    }
	
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

	[[nodiscard]] AModelSelection<AString> getSelectionModel() const {
	    return AModelSelection<AString>(mSelectionModel, mModel.get());
	}

    void setSize(int width, int height) override;

signals:
	emits<AModelSelection<AString>> selectionChanged;
};
