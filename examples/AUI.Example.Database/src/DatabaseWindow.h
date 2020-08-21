#pragma once
#include "AUI/Platform/AWindow.h"
#include "AUI/Data/ASqlDatabase.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ATableView.h"
#include "AUI/View/ATextField.h"

class DatabaseWindow: public AWindow
{
private:
	_<ASqlDatabase> mDB;
	_<ATableView> mTableView;
	_<ATextField> mInput;
	_<AButton> mExecuteButton;
	
	void execute();
	
public:
	explicit DatabaseWindow(const _<ASqlDatabase>& sql_database);
};
