#include "Discovery.h"

#include <AUI/Platform/AProcess.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <thread>

namespace aui_remote {

namespace {

AString scalarToString(const AJson& json) {
    if (json.isString()) {
        return json.asString();
    }
    if (json.isLongInt()) {
        return AString::number(std::holds_alternative<int>(json) ? std::get<int>(json) : std::get<int64_t>(json));
    }
    if (json.isNumber()) {
        return AString::number(json.asNumber());
    }
    return {};
}

AString readFile(const APath& path) {
    std::ifstream in(path.toStdString(), std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return AString::fromUtf8(ss.str());
}

}   // namespace

AString AppInfo::displayName() const {
    auto name = APath(exe).filename();
    return "{} ({})"_format(name.empty() ? AString("?") : AString(name), pid);
}

bool AppInfo::isSupported() const { return scheme == "wss" && certSha256.length() == 64; }

AppInfo AppInfo::fromJson(const AJson& json) {
    AppInfo info;
    if (auto v = json.containsOpt("protocol")) {
        info.protocol = v->asInt();
    }
    info.pid = static_cast<std::uint32_t>(json["pid"].asInt());
    info.port = static_cast<std::uint16_t>(json["port"].asInt());
    if (info.pid == 0 || info.port == 0) {
        throw AException("discovery file has no pid or port");
    }
    if (auto v = json.containsOpt("host"); v && v->isString()) {
        info.host = v->asString();
    }
    if (auto v = json.containsOpt("remote"); v && v->isBool()) {
        info.remote = v->asBool();
    }
    if (auto v = json.containsOpt("exe"); v && v->isString()) {
        info.exe = v->asString();
    }
    if (auto v = json.containsOpt("cwd"); v && v->isString()) {
        info.cwd = v->asString();
    }
    if (auto v = json.containsOpt("scheme"); v && v->isString()) {
        info.scheme = v->asString();
    }
    if (auto v = json.containsOpt("certSha256"); v && v->isString()) {
        info.certSha256 = AString(v->asString().lowercase());
    }
    if (auto v = json.containsOpt("defaultPointer"); v && v->isString()) {
        info.defaultPointer = v->asString();
    }
    if (auto v = json.containsOpt("pairing"); v && v->isString()) {
        info.pairing = v->asString();
    }
    if (auto v = json.containsOpt("startedAt")) {
        info.startedAt = scalarToString(*v);
    }
    if (auto v = json.containsOpt("argv"); v && v->isArray()) {
        for (const auto& arg : v->asArray()) {
            info.argv << scalarToString(arg);
        }
    }
    return info;
}

APath discovery::defaultDirectory() { return APath::getDefaultPath(APath::HOME) / ".aui" / "remote"; }

bool discovery::isProcessAlive(std::uint32_t pid) { return AProcess::fromPid(pid) != nullptr; }

bool discovery::terminateProcess(std::uint32_t pid, std::chrono::milliseconds grace) {
    auto process = AProcess::fromPid(pid);
    if (!process) {
        return true;
    }
    process->terminate();
    const auto deadline = std::chrono::steady_clock::now() + grace;
    while (isProcessAlive(pid) && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (isProcessAlive(pid)) {
        process->kill();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return !isProcessAlive(pid);
}

AVector<AppInfo> discovery::list(const APath& directory, const std::function<bool(std::uint32_t)>& isAlive) {
    AVector<AppInfo> result;
    if (!directory.isDirectoryExists()) {
        return result;
    }
    for (const auto& file : directory.listDir(AFileListFlags::DEFAULT_FLAGS)) {
        if (file.extension() != "json") {
            continue;
        }
        AppInfo info;
        try {
            info = AppInfo::fromJson(AJson::fromString(readFile(file)));
        } catch (const AException&) {
            continue;
        }
        if (!info.remote && !isAlive(info.pid)) {
            file.removeFile();
            continue;
        }
        result << std::move(info);
    }
    std::sort(result.begin(), result.end(), [](const AppInfo& a, const AppInfo& b) { return a.pid < b.pid; });
    return result;
}

void discovery::store(const AppInfo& app, const APath& directory) {
    directory.makeDirs();
    AJson argv = AJson::Array {};
    for (const auto& arg : app.argv) {
        argv.push_back(arg);
    }
    const AJson json {
        { "protocol", app.protocol },
        { "pid", int(app.pid) },
        { "port", int(app.port) },
        { "host", app.host },
        { "remote", true },
        { "scheme", app.scheme },
        { "certSha256", app.certSha256 },
        { "exe", app.exe },
        { "defaultPointer", app.defaultPointer },
        { "pairing", app.pairing },
        { "argv", std::move(argv) },
        { "cwd", app.cwd },
        { "startedAt", app.startedAt },
    };
    std::ofstream((directory / "remote-{}.json"_format(app.pid)).toStdString(), std::ios::binary)
        << AJson::toString(json).toStdString();
}

bool discovery::forget(std::uint32_t pid, const APath& directory) {
    auto file = directory / "remote-{}.json"_format(pid);
    if (!file.isRegularFileExists()) {
        return false;
    }
    file.removeFile();
    return true;
}

AOptional<AppInfo> discovery::find(std::uint32_t pid, const APath& directory) {
    auto file = directory / (AString::number(pid) + ".json");
    if (!file.isRegularFileExists()) {
        return std::nullopt;
    }
    return AppInfo::fromJson(AJson::fromString(readFile(file)));
}

}   // namespace aui_remote
