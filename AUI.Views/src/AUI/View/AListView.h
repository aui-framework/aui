#pragma once

#include <AUI/Model/AModelSelection.h>
#include "AViewContainer.h"
#include "AUI/Model/AModelIndex.h"
#include "AUI/Model/IListModel.h"

class AListItem;

class API_AUI_VIEWS AListView: public AViewContainer
{
private:
	_<IListModel<AVariant>> mModel;
	ASet<AModelIndex> mSelectionModel;
	
public:
    AListView(): AListView(nullptr) {}
	explicit AListView(const _<IListModel<AVariant>>& model);
    virtual ~AListView();

    void setModel(const _<IListModel<AVariant>>& model);

    int getContentMinimumHeight() override;
	
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

	[[nodiscard]] AModelSelection<AVariant> getSelectionModel() const {
	    return AModelSelection<AVariant>(mSelectionModel, mModel.get());
	}


signals:
	emits<AModelSelection<AVariant>> selectionChanged;
};
