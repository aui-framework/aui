/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


#include "AUI/Util/AMimedData.h"

class ASurface;

/**
 * @brief Utility class for creating and processing drag-n-drop events.
 * @ingroup views
 * @details
 *
 * Platform table
 * <table>
 *   <tr>
 *     <td>Windows</td>
 *     <td>Supported</td>
 *   </tr>
 *   <tr>
 *     <td>Linux</td>
 *     <td>-</td>
 *   </tr>
 *   <tr>
 *     <td>macOS</td>
 *     <td>-</td>
 *   </tr>
 *   <tr>
 *     <td>Android</td>
 *     <td>n/a</td>
 *   </tr>
 *   <tr>
 *     <td>iOS</td>
 *     <td>n/a</td>
 *   </tr>
 * </table>
 */
class API_AUI_VIEWS ADragNDrop {
public:
    ADragNDrop() = default;

    void setData(AMimedData data) {
        mData = std::move(data);
    }

    /**
     * @brief Pushes out a drag-n-drop operation to the OS.
     */
    void perform(ASurface* sourceWindow);


    struct EnterEvent {
        const AMimedData& data;
        glm::ivec2 mousePosition;
    };

    struct DropEvent {
        const AMimedData& data;
        glm::ivec2 mousePosition;
    };

private:
    ASurface* mWindow;
    AMimedData mData;
};


