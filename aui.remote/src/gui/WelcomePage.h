#pragma once

#include <AUI/Common/AProperty.h>
#include <AUI/View/AViewContainer.h>

#include "remote/Discovery.h"

/**
 * @brief The first tab: the switch that installs the skill for AI agents and the list of available connections.
 */
class WelcomePage : public AViewContainer {
public:
    WelcomePage();

    /**
     * @brief Replaces the list of connections. Does nothing if neither the applications, their pairing nor the open
     * tabs changed.
     * @param openPids applications that have a tab already
     */
    void setApps(const AVector<aui_remote::AppInfo>& apps, const AVector<std::uint32_t>& openPids);

signals:
    /**
     * @brief The user wants to pair with an application that does not know this client yet. No tab exists for it.
     */
    emits<aui_remote::AppInfo> pairRequested;

    /**
     * @brief Opens the tab of the application, connecting if it is not open yet.
     */
    emits<aui_remote::AppInfo> openRequested;

private:
    enum class Action {
        PAIR,      // never paired: asks for the code
        CONNECT,   // has a key: connects directly
        OPEN,      // the tab exists
    };
    struct Row {
        aui_remote::AppInfo app;
        Action action;
        bool operator==(const Row&) const = default;
    };
    AProperty<AVector<Row>> mRows;
    AProperty<bool> mSkillInstalled = false;
    AProperty<AString> mSkillStatus;

    [[nodiscard]] _<AView> connectionRow(const Row& row);
    void skillToggled(bool wanted);
};
