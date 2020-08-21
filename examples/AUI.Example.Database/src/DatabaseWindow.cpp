#include "DatabaseWindow.h"

#include "AUI/Data/ASqlTable.h"
#include "AUI/View/AListView.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ATableView.h"
#include "AUI/View/ATextField.h"

DatabaseWindow::DatabaseWindow(const _<ASqlDatabase>& sql_database):
		AWindow(u8"База данных", 900, 600),
		mDB(sql_database)
{
	auto tables = mDB->query("show tables;");

	setLayout(_new<AHorizontalLayout>());

	auto tableList = _new<AStringVector>();
	
	for (auto& e : *tables)
	{
		(*tableList) << e->getValue(0).toString();
	}

	auto tablesListView = _new<AListView>(tableList);
	connect(tablesListView->itemSelected, this, [&, tableList](const AModelIndex& selected)
	{
		auto selectedTableName = tableList->listItemAt(selected.getRow());
		mInput->setText("SELECT * FROM " + selectedTableName.toString());
		execute();
	});
	addView(tablesListView);
	addView(_container<AVerticalLayout>({
		_container<AHorizontalLayout>({
			mInput = _new<ATextField>(),
			mExecuteButton = _new<AButton>(u8"Выполнить запрос")
		}),
		mTableView = _new<ATableView>(),
	}));

	mInput->setExpanding({ 1, 1 });

	connect(mExecuteButton->clicked, this, &DatabaseWindow::execute);
}

void DatabaseWindow::execute()
{
	auto sqlInput = mInput->getText();
	mTableView->setModel(_new<ASqlTable>(mDB->query(sqlInput)));
}