#pragma once

#include <functional>

#include <AUI/View/AViewContainer.h>

#include "network/Discovery.h"

/**
 * @brief Contents of the modal window where the user enters the 6-digit code shown by the application.
 * @details
 * Asks the application to show the code on creation. `onPaired` is called when the application accepted the code and
 * the key is stored; the surface of the contents is closed afterwards.
 */
_<AViewContainer> pairingDialog(aui_remote::AppInfo app, std::function<void()> onPaired);
