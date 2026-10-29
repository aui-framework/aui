#include "Args.h"

#include <set>

namespace aui_remote::cli {

namespace {

const std::set<std::string> FLAGS = { "interactive", "include-invisible", "inspector", "json", "help" };

/// number if the text looks like one, string otherwise.
AJson numberOrString(const AString& text) {
    if (auto i = text.toInt()) {
        return *i;
    }
    return text;
}

AJson number(const AString& text, const AString& what) {
    if (auto i = text.toInt()) {
        return *i;
    }
    if (auto d = text.toDouble()) {
        return *d;
    }
    throw ArgsError("{} must be a number, got '{}'"_format(what, text));
}

int intOption(const ParsedArgs& args, const AString& name, int defaultValue) {
    if (auto v = args.get(name)) {
        if (auto i = v->toInt()) {
            return *i;
        }
        throw ArgsError("--{} must be an integer, got '{}'"_format(name, *v));
    }
    return defaultValue;
}

void copyWindow(const ParsedArgs& args, AJson& params) {
    if (auto w = args.get("window")) {
        params["windowId"] = numberOrString(*w);
    }
}

void requirePositional(const ParsedArgs& args, std::size_t count, const AString& usage) {
    if (args.positional.size() < count) {
        throw ArgsError("usage: aui-remote cli " + usage);
    }
}

AJson filters(const ParsedArgs& args) {
    AJson params = AJson::Object {};
    if (auto v = args.get("type")) {
        params["type"] = *v;
    }
    if (auto v = args.get("text")) {
        params["text"] = *v;
    }
    if (auto v = args.get("ass")) {
        params["assName"] = *v;
    }
    if (auto v = args.get("ref")) {
        params["ref"] = *v;
    }
    if (args.has("interactive")) {
        params["interactive"] = true;
    }
    if (args.has("include-invisible")) {
        params["includeInvisible"] = true;
    }
    copyWindow(args, params);
    return params;
}

/// Reads a pointer target (`<ref>` or `<x> <y>`) from the positional arguments starting at `index`.
void readTarget(const ParsedArgs& args, std::size_t& index, AJson& params, const AString& refKey, const AString& xKey,
                const AString& yKey, const AString& usage) {
    if (index >= args.positional.size()) {
        throw ArgsError("usage: aui-remote cli " + usage);
    }
    const auto& first = args.positional[index];
    if (first.toDouble()) {
        if (index + 1 >= args.positional.size()) {
            throw ArgsError("a point needs both x and y; usage: aui-remote cli " + usage);
        }
        params[xKey] = number(first, "x");
        params[yKey] = number(args.positional[index + 1], "y");
        index += 2;
    } else {
        params[refKey] = first;
        index += 1;
    }
}

AJson pointerParams(const ParsedArgs& args, const AString& action, const AString& usage) {
    AJson params = AJson::Object {};
    params["action"] = action;
    std::size_t index = 0;
    readTarget(args, index, params, "ref", "x", "y", usage);
    if (action == "drag") {
        readTarget(args, index, params, "toRef", "toX", "toY", usage);
    }
    if (action == "scroll") {
        params["dx"] = intOption(args, "dx", 0);
        params["dy"] = intOption(args, "dy", 0);
    }
    if (auto b = args.get("button")) {
        params["button"] = *b;
    }
    if (args.has("mod")) {
        AJson modifiers = AJson::Array {};
        for (const auto& m : args.getAll("mod")) {
            modifiers.push_back(m);
        }
        params["modifiers"] = std::move(modifiers);
    }
    copyWindow(args, params);
    return params;
}

constexpr std::chrono::milliseconds WAIT_MARGIN = std::chrono::seconds(10);

}   // namespace

AOptional<AString> ParsedArgs::get(const AString& name) const {
    if (auto it = options.find(name); it != options.end() && !it->second.empty()) {
        return it->second.last();
    }
    return std::nullopt;
}

AStringVector ParsedArgs::getAll(const AString& name) const {
    if (auto it = options.find(name); it != options.end()) {
        return it->second;
    }
    return {};
}

AOptional<int> ParsedArgs::getInt(const AString& name) const {
    if (auto v = get(name)) {
        return v->toInt();
    }
    return std::nullopt;
}

bool ParsedArgs::isFlag(const AString& name) { return FLAGS.contains(name.toStdString()); }

ParsedArgs ParsedArgs::parse(const AStringVector& args) {
    ParsedArgs result;
    bool commandRead = false;
    for (std::size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];
        if (arg == "--") {
            for (++i; i < args.size(); ++i) {
                result.rest << args[i];
            }
            break;
        }
        if (arg.startsWith("--") && arg.length() > 2) {
            auto name = arg.substr(2);
            if (auto eq = name.find('='); eq != AString::NPOS) {
                result.options[name.substr(0, eq)] << name.substr(eq + 1);
            } else if (isFlag(name)) {
                result.options[name];
            } else {
                if (i + 1 >= args.size()) {
                    throw ArgsError("option --{} needs a value"_format(name));
                }
                result.options[name] << args[++i];
            }
            continue;
        }
        if (!commandRead) {
            result.command = arg;
            commandRead = true;
        } else {
            result.positional << arg;
        }
    }
    return result;
}

AStringVector commandNames() {
    return { "list",     "pair",  "hello",    "windows", "resize", "tree",      "find",     "props",     "hit",
             "screenshot", "interactive", "click", "dblclick", "press", "release", "drag",       "scroll",
             "move",     "type",  "key",      "wait-idle", "wait-view", "logs",     "functions", "call",
             "launch",   "stop",  "compare" };
}

AOptional<CommandRequest> buildRequest(const ParsedArgs& args) {
    const auto& cmd = args.command;
    CommandRequest request;
    request.params = AJson::Object {};
    auto& params = request.params;

    if (cmd == "hello") {
        request.method = "session.hello";
    } else if (cmd == "windows") {
        request.method = "window.list";
    } else if (cmd == "resize") {
        requirePositional(args, 2, "resize <width> <height> [--window id] [--timeout ms]");
        request.method = "window.resize";
        for (auto [i, name] : { std::pair { 0, "width" }, std::pair { 1, "height" } }) {
            auto value = args.positional[i].toInt();
            if (!value || *value <= 0) {
                throw ArgsError("{} must be a positive integer, got '{}'"_format(name, args.positional[i]));
            }
            params[name] = *value;
        }
        if (args.has("timeout")) {
            const int timeout = intOption(args, "timeout", 0);
            params["timeoutMs"] = timeout;
            request.timeout = std::chrono::milliseconds(timeout) + WAIT_MARGIN;
        }
        copyWindow(args, params);
    } else if (cmd == "tree") {
        request.method = "view.tree";
        if (auto v = args.get("ref")) {
            params["ref"] = *v;
        }
        if (args.has("depth")) {
            params["depth"] = intOption(args, "depth", 0);
        }
        if (args.has("inspector")) {
            params["include"] = AJson::Array { "inspector" };
        }
        copyWindow(args, params);
    } else if (cmd == "find") {
        request.method = "view.find";
        params = filters(args);
    } else if (cmd == "props") {
        requirePositional(args, 1, "props <ref>");
        request.method = "view.properties";
        params["ref"] = args.positional[0];
    } else if (cmd == "hit") {
        requirePositional(args, 2, "hit <x> <y> [--window id]");
        request.method = "view.hitTest";
        params["x"] = number(args.positional[0], "x");
        params["y"] = number(args.positional[1], "y");
        copyWindow(args, params);
    } else if (cmd == "interactive") {
        request.method = "view.interactive";
        copyWindow(args, params);
    } else if (cmd == "screenshot") {
        request.method = "screenshot";
        if (auto v = args.get("ref")) {
            params["ref"] = *v;
        }
        if (args.has("padding")) {
            params["padding"] = intOption(args, "padding", 0);
        }
        copyWindow(args, params);
    } else if (cmd == "click" || cmd == "dblclick" || cmd == "press" || cmd == "release" || cmd == "move" ||
               cmd == "drag" || cmd == "scroll") {
        request.method = "input.pointer";
        const AString usage = cmd == "drag"     ? "drag <ref|x y> <ref|x y> [--button b] [--mod m]..."
                              : cmd == "scroll" ? "scroll <ref|x y> [--dx n] [--dy n]"
                                                : cmd + " <ref|x y> [--button b] [--mod m]...";
        params = pointerParams(args, cmd == "dblclick" ? AString("doubleClick") : cmd, usage);
    } else if (cmd == "type") {
        requirePositional(args, 1, "type <text> [--ref ref]");
        request.method = "input.type";
        params["text"] = args.positional[0];
        if (auto v = args.get("ref")) {
            params["ref"] = *v;
        }
    } else if (cmd == "key") {
        requirePositional(args, 1, "key <key> [--mod ctrl]... [--ref ref] [--action press|release]");
        request.method = "input.key";
        params["key"] = args.positional[0];
        if (args.has("mod")) {
            AJson modifiers = AJson::Array {};
            for (const auto& m : args.getAll("mod")) {
                modifiers.push_back(m);
            }
            params["modifiers"] = std::move(modifiers);
        }
        if (auto v = args.get("ref")) {
            params["ref"] = *v;
        }
        if (auto v = args.get("action")) {
            params["action"] = *v;
        }
    } else if (cmd == "wait-idle") {
        request.method = "wait.idle";
        const int timeout = intOption(args, "timeout", 0);
        if (args.has("timeout")) {
            params["timeoutMs"] = timeout;
            request.timeout = std::chrono::milliseconds(timeout) + WAIT_MARGIN;
        }
        if (args.has("quiet")) {
            params["quietMs"] = intOption(args, "quiet", 0);
        }
    } else if (cmd == "wait-view") {
        request.method = "wait.view";
        params = filters(args);
        params["state"] = args.get("state").valueOr("visible");
        if (params["state"].asString() != "visible" && params["state"].asString() != "gone") {
            throw ArgsError("--state must be visible or gone");
        }
        if (args.has("timeout")) {
            const int timeout = intOption(args, "timeout", 0);
            params["timeoutMs"] = timeout;
            request.timeout = std::chrono::milliseconds(timeout) + WAIT_MARGIN;
        }
    } else if (cmd == "logs") {
        request.method = "diag.log";
        if (args.has("since")) {
            params["since"] = intOption(args, "since", 0);
        }
        if (auto v = args.get("level")) {
            params["level"] = *v;
        }
        if (args.has("limit")) {
            params["limit"] = intOption(args, "limit", 0);
        }
    } else if (cmd == "functions") {
        request.method = "fn.list";
    } else if (cmd == "call") {
        requirePositional(args, 1, "call <name> [--args '{\"json\":true}']");
        request.method = "fn.call";
        params["name"] = args.positional[0];
        if (auto v = args.get("args")) {
            try {
                params["arguments"] = AJson::fromString(*v);
            } catch (const AException& e) {
                throw ArgsError("--args is not valid JSON: " + e.getMessage());
            }
        } else {
            params["arguments"] = AJson::Object {};
        }
    } else {
        return std::nullopt;
    }
    return request;
}

}   // namespace aui_remote::cli
