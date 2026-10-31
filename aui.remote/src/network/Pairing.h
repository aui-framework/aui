#pragma once

#include <AUI/Common/AString.h>

#include "Discovery.h"
#include "KeyStore.h"

namespace aui_remote::pairing {

/**
 * @brief What `confirm` found out about the pairing.
 */
struct Result {
    /**
     * @brief The application remembers the key: no code is needed again, even after the application restarts.
     */
    bool remembered = false;

    /**
     * @brief The key works on the connection that has paired only (the application asks for a code every time).
     */
    bool connectionOnly = false;
};

/**
 * @brief Sends the code to the application together with a new key and keeps the key when the application accepts it.
 * @details
 * Where the key goes depends on what the application says it will do with it: the paired key if it remembers it, the
 * session key if it trusts the client until it exits, nowhere if it asks for the code on every connection.
 * @throws RpcError if the application refuses the code or can't be reached
 */
Result confirm(const AppInfo& app, const AString& code, const KeyStore& store);

}   // namespace aui_remote::pairing
