#pragma once

#include <AUI/Platform/AWindow.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>

class MainWindow: public AWindow {
private:
    _<AButton> mDeleteButton;
    _<AListView> mList;

public:

    MainWindow();
};