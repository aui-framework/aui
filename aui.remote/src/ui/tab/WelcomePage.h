#pragma once

#include <functional>

#include <AUI/Common/AProperty.h>
#include <AUI/View/AView.h>

#include "network/Discovery.h"

/**
 * @brief An application in the list of available connections.
 */
struct ConnectionRow {
    enum class Action {
        PAIR,      // never paired: asks for the code
        CONNECT,   // has a key: connects directly
        OPEN,      // the tab exists
    };
    aui_remote::AppInfo app;
    Action action;
    bool operator==(const ConnectionRow&) const = default;
};

/**
 * @brief The first tab: the switch that installs the skill for AI agents and the list of available connections.
 * @param rows the list of connections
 * @param onPair the user wants to pair with an application that does not know this client yet; no tab exists for it
 * @param onOpen opens the tab of the application, connecting if it is not open yet
 */
_<AView> welcomePage(_<AProperty<AVector<ConnectionRow>>> rows, std::function<void(aui_remote::AppInfo)> onPair,
                     std::function<void(aui_remote::AppInfo)> onOpen);
