#include "ExampleWindow.h"

#include "AUI/Curl/ACurl.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/View/AButton.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Platform/ACustomCaptionWindow.h"
#include "AUI/View/ALabel.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/Network/ATcpSocket.h"
#include "AUI/Platform/AMessageBox.h"
#include "AUI/Url/AUrl.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/View/ASpacer.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/Data/ASqlDatabase.h"
#include "DatabaseWindow.h"

ExampleWindow::ExampleWindow() : AWindow(u8"����������� � ��")
{
	setWindowStyle(WS_DIALOG);

	setLayout(_new<AVerticalLayout>());
	addCssName(".window-no-padding");

	auto address = _new<ATextField>();
	auto port = _new<ANumberPicker>();
	auto dbName = _new<ATextField>();
	auto username = _new<ATextField>();
	auto password = _new<ATextField>();

	address->setText("alex2772.ru");
	dbName->setText("fgs");
	username->setText("fgs");
	password->setText("xnst2912a2");

	port->setMin(100);
	port->setMax(0xffff);
	port->setValue(3306);

	auto form = _form({
		//{u8"�����", address},
		//{u8"����", port},
		//{u8"�������� ��", dbName},
		//{u8"��� ������������", username},
		//{u8"������", password},
	});
	form->addCssName(".window-content");

	addView(form);

	auto connectButton = _new<AButton>(u8"������������");
	connectButton->setDefault();
	auto cancelButton = _new<AButton>(u8"������");

	auto buttons = _container<AHorizontalLayout>({
		_new<ASpacer>(),
		connectButton,
		cancelButton
	});

	buttons->addCssName(".dialog-button-box");

	connect(cancelButton->clicked, this, &ExampleWindow::quit);
	connect(connectButton->clicked, this, [=]()
	{
		try {
			auto db = ASqlDatabase::connect("mysql", address->getText(), port->getValue(), dbName->getText(),
				username->getText(), password->getText());

			quit();
			_new<DatabaseWindow>(db)->loop();

		} catch (const AException& e)
		{
			AMessageBox::show(this, u8"�� ������� ������������ � ��", e.getMessage(), AMessageBox::I_CRITICAL);
		}
	});

	addView(buttons);

	pack();
}
