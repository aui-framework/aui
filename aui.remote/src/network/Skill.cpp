#include "Skill.h"

#include "cli/Args.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

#include <AUI/IO/AByteBufferInputStream.h>
#include <AUI/Url/AUrl.h>

namespace aui_remote::skill {

APath defaultFile() {
    APath root;
    if (const char* configDir = std::getenv("CLAUDE_CONFIG_DIR"); configDir != nullptr && *configDir != '\0') {
        root = APath(configDir);
    } else {
        root = APath::getDefaultPath(APath::HOME) / ".claude";
    }
    return root / "skills" / "aui-remote" / "SKILL.md";
}

AString builtinTemplate() {
    auto stream = AUrl(":skill/SKILL.md").open();
    std::string result;
    char buf[4096];
    while (auto n = stream->read(buf, sizeof(buf))) {
        result.append(buf, n);
    }
    return AString::fromUtf8(result);
}

AString render(const AString& templateText, const APath& exe) {
    return templateText.replacedAll("{{COMMANDS}}", cli::commandHelpTable()).replacedAll("{{EXE}}", exe);
}

bool isInstalled(const APath& file) { return file.isRegularFileExists(); }

void install(const AString& templateText, const APath& exe, const APath& file) {
    file.parent().makeDirs();
    std::ofstream out(file.toStdString(), std::ios::binary | std::ios::trunc);
    out << render(templateText, exe).toStdString();
    if (!out) {
        throw AException("can't write {}"_format(file));
    }
}

void uninstall(const APath& file) {
    if (file.isRegularFileExists()) {
        file.removeFile();
    }
    if (auto folder = file.parent(); folder.isDirectoryExists() && folder.listDir().empty()) {
        folder.removeFile();
    }
}

}   // namespace aui_remote::skill
