#pragma once
#include "AViewContainer.h"
#include "AUI/Model/AModelIndex.h"

class AListItem;

class API_AUI_VIEWS AListView: public AViewContainer
{
private:
	_<IListModel> mModel;

	_<AListItem> mSelected;
	
public:
	AListView(const _<IListModel>& model);

	int getContentMinimumHeight() override;
	
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
	virtual ~AListView();

signals:
	emits<AModelIndex> itemSelected;
};
