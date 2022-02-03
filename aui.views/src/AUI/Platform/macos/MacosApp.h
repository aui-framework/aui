//
// Created by Alexey Titov on 22.01.2022.
//
#pragma once

class MacosApp {
private:
    MacosApp();
    void* mNsApp;
public:
    static MacosApp& inst();

    void run();
    void activateIgnoringOtherApps();

    void quit();
};

