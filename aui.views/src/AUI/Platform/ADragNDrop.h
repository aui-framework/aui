// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once


#include "AUI/Util/AMimedData.h"

class ABaseWindow;

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
    void perform(ABaseWindow* sourceWindow);


    struct EnterEvent {
        const AMimedData& data;
        glm::ivec2 mousePosition;
    };

    struct DropEvent {
        const AMimedData& data;
        glm::ivec2 mousePosition;
    };

private:
    ABaseWindow* mWindow;
    AMimedData mData;
};


