#pragma once


#include "ABaseWindow.h"
#include "AUI/Util/AMimedData.h"

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

private:
    ABaseWindow* mWindow;
    AMimedData mData;
};


