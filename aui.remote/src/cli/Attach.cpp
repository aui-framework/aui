#include "Attach.h"

#include <AUI/Common/AException.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Platform/AProcess.h>

#include <chrono>
#include <cstdlib>
#include <thread>

#include "network/RpcClient.h"

using namespace std::chrono_literals;

namespace aui_remote::cli {

namespace {

constexpr auto LOGCAT_TAG = "ARemoteDebug";
constexpr std::chrono::seconds ADB_TIMEOUT = 20s;

RpcError error(AString message, AString hint) {
    return RpcError(rpc_code::CLIENT_ERROR, std::move(message), AJson { { "hint", std::move(hint) } });
}

APath findAdb(const ParsedArgs& args) {
    if (auto path = args.get("adb-path")) {
        return *path;
    }
    if (const char* adb = std::getenv("ADB")) {
        return adb;
    }
    for (const char* sdk : { "ANDROID_HOME", "ANDROID_SDK_ROOT" }) {
        if (const char* root = std::getenv(sdk)) {
            if (const auto adb = APath(root) / "platform-tools" / "adb"; adb.isRegularFileExists()) {
                return adb;
            }
        }
    }
    if (auto found = APath::find("adb", {}, APathFinder::USE_SYSTEM_PATHS); !found.empty()) {
        return found.first();
    }
    throw error("adb is not found", "pass --adb-path, set ADB or ANDROID_HOME, or put adb into PATH");
}

/// adb ends its lines with \n (\r\n on Windows), which AString::trim leaves.
AString trimmed(const AString& text) {
    constexpr auto SPACES = " \t\r\n";
    const auto string = text.toStdString();
    const auto first = string.find_first_not_of(SPACES);
    if (first == std::string::npos) {
        return {};
    }
    return AString::fromUtf8(string.substr(first, string.find_last_not_of(SPACES) - first + 1));
}

/// Runs `adb [-s serial] <arguments>` and returns its stdout.
AString adb(const APath& executable, const AOptional<AString>& serial, std::initializer_list<AString> arguments) {
    const auto output = APath::getDefaultPath(APath::TEMP) / "aui-remote" / "adb-stdout.txt";
    output.parent().makeDirs();
    AStringVector all;
    if (serial) {
        all << "-s" << *serial;
    }
    all.insertAll(AStringVector(arguments));

    AProcess::ProcessCreationInfo info;
    info.executable = executable;
    info.args = AProcess::ArgStringList { std::move(all) };
    info.stdOutFile = output;
    auto process = AProcess::create(std::move(info));
    process->run();
    // not waitForExitCode(): it has no timeout, and adb may wait for a device that never comes
    const auto deadline = std::chrono::steady_clock::now() + ADB_TIMEOUT;
    while (!process->isFinished()) {
        if (std::chrono::steady_clock::now() > deadline) {
            process->kill();
            throw error("adb {} did not finish in {} s"_format(AStringVector(arguments).join(' '), ADB_TIMEOUT.count()),
                        "check `adb devices`: the device may be offline or unauthorized");
        }
        std::this_thread::sleep_for(20ms);
    }
    const auto code = process->exitCodeNonBlocking().valueOr(-1);
    const auto text = AString::fromUtf8(AByteBuffer::fromStream(AFileInputStream(output)));
    if (code != 0) {
        throw error("adb {} failed ({}): {}"_format(AStringVector(arguments).join(' '), code, trimmed(text)),
                    "check `adb devices`; with several devices pass --serial");
    }
    return text;
}

/// The last `discovery {...}` line the agent wrote to logcat.
AJson lastDiscoveryFromLogcat(const AString& logcat) {
    AOptional<AString> json;
    for (const auto& line : logcat.split('\n')) {
        if (const auto at = line.find("discovery {"); at != AString::NPOS) {
            json = trimmed(line.substr(at + std::string_view("discovery ").size()));
        }
    }
    if (!json) {
        throw error("no remote agent has written its discovery to logcat",
                    "the application must link aui::dev_extras and call ARemoteDebug::start(); start it and retry");
    }
    return AJson::fromString(*json);
}

/// The address of the device in the local network.
AString deviceAddress(const APath& executable, const AOptional<AString>& serial) {
    const auto route = adb(executable, serial, { "shell", "ip", "route", "get", "1.1.1.1" });
    const auto at = route.find(" src ");
    if (at == AString::NPOS) {
        throw error("the device has no route to a network", "connect it to Wi-Fi, or attach without --lan");
    }
    const auto rest = route.substr(at + 5);
    return trimmed(rest.substr(0, rest.find(' ')));
}

AppInfo storeTarget(AppInfo app, const Directories& directories) {
    if (!isAgentReachable(app)) {
        throw error("{}:{} does not answer"_format(app.host, app.port),
                    app.host == "127.0.0.1"
                        ? "the application may have been restarted: run `attach` again; check the logcat of the device"
                        : "the application has to listen on the network (AUI_REMOTE_BIND=0.0.0.0, on Android `adb "
                          "shell setprop debug.aui.remote.bind 0.0.0.0` before the start) and the firewall "
                          "has to let the port in");
    }
    discovery::store(app, directories.discovery);
    return app;
}

AJson describe(const AppInfo& app) {
    return AJson {
        { "pid", int(app.pid) }, { "host", app.host }, { "port", int(app.port) }, { "exe", app.exe },
        { "hint", "now run `pair {0}` (the code is on the screen of the application), then use the commands with "
                  "--pid {0}"_format(app.pid) },
    };
}

}   // namespace

bool isAgentReachable(const AppInfo& app) {
    try {
        (void) connectToAgent(app.host, app.port, app.certSha256, "/", 3s);
        return true;
    } catch (const AException&) {
        return false;
    }
}

AJson doAttach(const ParsedArgs& args, const Directories& directories) {
    if (!args.has("adb")) {
        const auto host = args.get("host");
        const auto port = args.getInt("port");
        const auto cert = args.get("cert");
        if (!host || !port || !cert) {
            throw ArgsError("usage: aui-remote cli attach --adb [--serial S] [--lan] | attach --host H --port P --cert "
                            "SHA256 [--pid N] [--name NAME]");
        }
        const auto pid = args.getInt("pid").valueOr(*port);
        AppInfo app;
        app.pid = pid;
        app.port = *port;
        app.host = *host;
        app.remote = true;
        app.scheme = "wss";
        app.certSha256 = AString(cert->lowercase());
        app.exe = args.get("name").valueOr("app@{}"_format(*host));
        app.startedAt = "manual";
        return describe(storeTarget(std::move(app), directories));
    }

    const auto executable = findAdb(args);
    auto serial = args.get("serial");
    if (!serial) {
        serial = trimmed(adb(executable, std::nullopt, { "get-serialno" }));
    }
    const auto discoveryJson = lastDiscoveryFromLogcat(adb(executable, serial, { "logcat", "-d", "-s", "{}:I"_format(LOGCAT_TAG) }));
    auto app = AppInfo::fromJson(discoveryJson);
    app.remote = true;
    // the key of a pairing is looked up by the executable: apps on devices have the same one (`app_process64`)
    app.exe = "{}@{}"_format(app.exe, *serial);
    if (args.has("lan")) {
        app.host = deviceAddress(executable, serial);
    } else {
        const auto forwarded = trimmed(adb(executable, serial, { "forward", "tcp:0", "tcp:{}"_format(app.port) }));
        const auto local = forwarded.toUInt();
        if (!local) {
            throw error("unexpected answer of adb forward: " + forwarded, "run `adb forward --list`");
        }
        app.port = static_cast<std::uint16_t>(*local);
    }
    return describe(storeTarget(std::move(app), directories));
}

AJson doDetach(const ParsedArgs& args, const Directories& directories) {
    if (args.positional.empty() || !args.positional[0].toUInt()) {
        throw ArgsError("usage: aui-remote cli detach <pid>");
    }
    const auto pid = *args.positional[0].toUInt();
    return AJson { { "detached", discovery::forget(pid, directories.discovery) }, { "pid", int(pid) } };
}

}   // namespace aui_remote::cli
