#pragma once
#include "AWindow.h"

#ifdef _WIN32

class API_AUI_VIEWS CustomWindow: public AWindow
{
protected:
	LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void doDrawWindow() override;
	
public:
	CustomWindow(const AString& name, int width, int height);
	CustomWindow();
	~CustomWindow() override;

	void setSize(int width, int height) override;
};

#else
class API_AUI_VIEWS CustomWindow: public AWindow
{
public:
    CustomWindow(const AString& name, int width, int height);
    CustomWindow() = default;
    ~CustomWindow() override = default;
};
#endif