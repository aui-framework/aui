#pragma once

#include <AUI/Common/AProperty.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Thread/AAsyncHolder.h>

#include "remote/Discovery.h"

/**
 * @brief Modal window where the user enters the 6-digit code shown by the application.
 * @details
 * Asks the application to show the code on creation. `paired` is emitted when the application accepted the code and the
 * key is stored.
 */
class PairingDialog : public AWindow {
public:
    PairingDialog(AWindow& parent, aui_remote::AppInfo app);

signals:
    emits<> paired;

private:
    aui_remote::AppInfo mApp;
    AProperty<AString> mCode;
    AProperty<AString> mStatus;
    AProperty<bool> mBusy = false;
    AAsyncHolder mAsync;

    void begin();
    void confirm();
};
