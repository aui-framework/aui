#pragma once

#include "AUI/Platform/AWindow.h"
/**
 * @brief Platform Abstraction Layer
 * @details
 * Runtime layer for switching between multiple displaying options (i.e, Linux = X11 or Wayland).
 *
 * This is a direct equivalent of Qt's Platform Abstraction.
 */
class IPlatformAbstraction {
public:
    virtual ~IPlatformAbstraction() = default;
    virtual _<ACursor::Custom> createCustomCursor(AImageView image) = 0;
    virtual void applyNativeCursor(const ACursor& cursor, AWindow* pWindow) = 0;

    static IPlatformAbstraction& current();
};