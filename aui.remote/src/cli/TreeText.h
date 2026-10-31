#pragma once

#include <AUI/Common/AString.h>
#include <AUI/Json/AJson.h>

namespace aui_remote {

/**
 * @brief Renders the result of `view.tree` as compact indented text, one view per line.
 * @details
 * ```
 * AWindow [v3:2] 900x450
 *   ALabel "Value: 0" [v4:9] 695x22 @102,170
 *   AButton "Click me" [v4:10] 695x33 @102,204 clickable
 * ```
 * A line is: the type, the text in quotes, the ref, the size, `@x,y` in the window, then `hidden`, `disabled`,
 * `focused` and the actions the view supports. It is a fraction of the size of the JSON, which also carries the
 * geometry relative to the parent, ASS names and so on.
 */
AString renderTreeText(const AJson& root);

}   // namespace aui_remote
