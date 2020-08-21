#include "ExampleWindow.h"

#include "AUI/Curl/ACurl.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/View/AButton.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Platform/CustomCaptionWindow.h"
#include "AUI/View/ALabel.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/Network/ATcpSocket.h"
#include "AUI/Platform/AMessageBox.h"
#include "AUI/Url/AUrl.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/View/ASpacer.h"

ExampleWindow::ExampleWindow(): AWindow(u8"Сеть")
{
	setLayout(_new<AVerticalLayout>());
	addCssName(".window-no-padding");

	auto horizontal = _new<AViewContainer>();
	horizontal->addCssName(".window-content");
	horizontal->setLayout(_new<AHorizontalLayout>());

	auto addr = _new<ATextField>();
	addr->setExpanding({ 1, 0 });

	auto label = _new<ALabel>();
	label->setExpanding({ 1, 1 });
	label->setMultiline(true);
	
	auto downloadASock = _new<AButton>(u8"Загрузить, используя ASocket");
	connect(downloadASock->clicked, this, [&, addr, downloadASock, label]()
	{
		auto input = addr->getText();
		auto p = input.find(':');
		if (p > 5 || p == AString::NPOS)
		{
			input = "http://" + input;
			addr->setText(input);
		}
		
		AUrl url(input);

		if (url.getProtocol() != "http")
		{
			AMessageBox::show(this, u8"AUI", u8"Поддерживается только протокол HTTP.");
			return;
		}
		
		downloadASock->setEnabled(false);
		auto socket = _new<ATcpSocket>(AInet4Address{url.getHost(), 80});
		mInput = _new<InputStreamAsync>(socket);

		(*socket) << AString("GET /") << url.getPath() << AString(" HTTP/1.0\r\n")
			      << AString("Host: ") << url.getHost() << AString("\r\n\r\n");

		
		label->setText("");

		connect(mInput->read, this, [label](_<ByteBuffer> buf)
		{
			label->setText(label->getText() + AString::fromLatin1(buf));
		});
		connect(mInput->finished, this, [downloadASock]()
		{
			downloadASock->setEnabled(true);
		});
	});
	auto downloadCurl = _new<AButton>(u8"Загрузить, используя cURL");
	connect(downloadCurl->clicked, this, [&, addr, downloadCurl, label]()
	{
		auto input = addr->getText();
		auto p = input.find(':');
		if (p > 5 || p == AString::NPOS)
		{
			input = "http://" + input;
			addr->setText(input);
		}
		
		
		auto socket = _new<ACurl>(input);
		mInput = _new<InputStreamAsync>(socket);

		
		label->setText("");

		connect(mInput->read, this, [label](_<ByteBuffer> buf)
		{
			label->setText(label->getText() + AString::fromLatin1(buf));
		});
		connect(mInput->finished, this, [downloadCurl]()
		{
			downloadCurl->setEnabled(true);
		});
	});
	
	horizontal->addView(addr);
	horizontal->addView(downloadASock);
	horizontal->addView(downloadCurl);
	
	addView(horizontal);

	label->setExpanding({ 1, 1 });
	addView(label);
}
