#pragma once
#include "AViewContainer.h"
#include "AUI/Model/ITableModel.h"

class API_AUI_VIEWS ATableView: public AViewContainer
{
private:
	_<ITableModel> mModel;
	
public:
	ATableView(const _<ITableModel>& model)
	{
		AVIEW_CSS;
		setModel(model);
	}
	
	ATableView()
	{
		AVIEW_CSS;
	}

	~ATableView() override;

	void setModel(_<ITableModel> model);
	
};
