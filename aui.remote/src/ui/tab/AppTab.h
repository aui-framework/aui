#pragma once

#include <functional>

#include <AUI/Common/AProperty.h>
#include <AUI/View/AView.h>

#include "network/Discovery.h"

/**
 * @brief What the tab strip knows about a connection.
 */
struct AppTabModel {
    enum class State {
        CONNECTING,
        NEEDS_PAIRING,
        READY,
        FAILED,   // not ERROR: windows.h defines it as a macro
        EXITED,
    };

    aui_remote::AppInfo app;
    AProperty<State> state = State::CONNECTING;

    /**
     * @brief Whether the application is still listed by discovery.
     */
    AProperty<bool> alive = true;

    /**
     * @brief `Local / myapp (1234)`
     */
    [[nodiscard]] AString title() const { return "Local / " + app.displayName(); }
};

/**
 * @brief Asks the user to pair with the application; the callback is called when they did.
 */
using PairRequest = std::function<void(const aui_remote::AppInfo&, std::function<void()> onPaired)>;

/**
 * @brief The page of a single connection: pairing, the live view of one application (frames, input and the view tree).
 */
_<AView> appTab(_<AppTabModel> model, PairRequest pair);
