#pragma once
#include "AViewContainer.h"
#include "AUI/Model/AModelIndex.h"
#include "AUI/Model/IListModel.h"

class AListItem;

class API_AUI_VIEWS AListView: public AViewContainer
{
private:
	_<IListModel<AVariant>> mModel;

	_<AListItem> mSelected;
	
public:
	explicit AListView(const _<IListModel<AVariant>>& model);
    virtual ~AListView();

	int getContentMinimumHeight() override;
	
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

signals:
	emits<AModelIndex> itemSelected;
};
