#pragma once

#include <AUI/Common/AString.h>
#include <AUI/IO/APath.h>

namespace aui_remote::skill {

/**
 * @return `($CLAUDE_CONFIG_DIR or ~/.claude)/skills/aui-remote/SKILL.md`
 */
APath defaultFile();

/**
 * @return the skill template that is built into the application (`assets/skill/SKILL.md`)
 */
AString builtinTemplate();

/**
 * @return the template with `{{EXE}}` replaced by the path to the executable
 */
AString render(const AString& templateText, const APath& exe);

/**
 * @return whether the skill file exists
 */
bool isInstalled(const APath& file = defaultFile());

/**
 * @brief Writes the rendered skill to `file`, creating the folders.
 */
void install(const AString& templateText, const APath& exe, const APath& file = defaultFile());

/**
 * @brief Deletes the skill file and its folder if it became empty.
 */
void uninstall(const APath& file = defaultFile());

}   // namespace aui_remote::skill
