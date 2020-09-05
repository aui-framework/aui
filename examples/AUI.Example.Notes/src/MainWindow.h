#pragma once

#include <AUI/Platform/AWindow.h>
#include <AUI/View/AButton.h>

class MainWindow: public AWindow {
private:
    _<AButton> mDeleteButton;

public:
    MainWindow();
};