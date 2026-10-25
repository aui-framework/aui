#include "Cli.h"

#include <chrono>
#include <iostream>
#include <thread>

#include <AUI/Image/AImage.h>
#include <AUI/Platform/AProcess.h>

#include "ImageCompare.h"
#include "Remote/KeyStore.h"
#include "Remote/RpcClient.h"

using namespace std::chrono_literals;

namespace aui_remote::cli {

namespace {

AJson toJson(const AppInfo& app, bool paired) {
    AJson argv = AJson::Array {};
    for (const auto& a : app.argv) {
        argv.push_back(a);
    }
    return AJson {
        { "pid", int(app.pid) },
        { "port", int(app.port) },
        { "exe", app.exe },
        { "argv", std::move(argv) },
        { "cwd", app.cwd },
        { "startedAt", app.startedAt },
        { "paired", paired },
    };
}

RpcError clientError(AString message, AString hint, AJson extra = {}) {
    AJson data = extra.isObject() ? extra : AJson(AJson::Object {});
    data["hint"] = std::move(hint);
    return RpcError(rpc_code::CLIENT_ERROR, std::move(message), std::move(data));
}

void print(const AJson& json) { std::cout << AJson::toString(json).toStdString() << std::endl; }

void note(const AString& text) { std::cerr << text.toStdString() << std::endl; }

AppInfo appByPid(std::uint32_t pid, const Directories& directories) {
    auto apps = discovery::list(directories.discovery);
    for (const auto& app : apps) {
        if (app.pid == pid) {
            return app;
        }
    }
    AJson candidates = AJson::Array {};
    for (const auto& app : apps) {
        candidates.push_back(int(app.pid));
    }
    throw clientError(
        "no running application with a remote agent has pid {}"_format(pid),
        "run `aui-remote cli list` to see the running applications", AJson { { "candidates", candidates } });
}

std::uint32_t pidArgument(const ParsedArgs& args) {
    AOptional<AString> text = args.get("pid");
    if (!text && !args.positional.empty()) {
        text = args.positional[0];
    }
    if (!text) {
        throw ArgsError("a pid is required: aui-remote cli {} <pid>"_format(args.command));
    }
    auto pid = text->toUInt();
    if (!pid) {
        throw ArgsError("pid must be a number, got '{}'"_format(*text));
    }
    return *pid;
}

AJson doList(const Directories& directories) {
    auto apps = discovery::list(directories.discovery);
    KeyStore store(directories.keys);
    store.pruneExcept(apps);
    AJson result = AJson::Array {};
    for (const auto& app : apps) {
        result.push_back(toJson(app, store.load(app).hasValue()));
    }
    return AJson { { "apps", std::move(result) } };
}

AJson doPair(const ParsedArgs& args, const Directories& directories) {
    const auto app = appByPid(pidArgument(args), directories);
    KeyStore store(directories.keys);
    if (auto code = args.get("code")) {
        const auto key = KeyStore::generateKey();
        auto result = RpcClient(app.port, "").call("pair.confirm", AJson { { "code", *code }, { "key", key } });
        if (!(result.isObject() && result.containsOpt("paired") && (result)["paired"].asBool())) {
            throw clientError("the application did not confirm the pairing", "call pair again", result);
        }
        store.store(app, key);
        return AJson { { "paired", true }, { "pid", int(app.pid) } };
    }
    RpcClient(app.port, "").call("pair.begin");
    note("The window of {} shows a 6-digit code. Finish with: aui-remote cli pair {} --code <code>"_format(
        app.displayName(), app.pid));
    return AJson {
        { "pending", true },
        { "pid", int(app.pid) },
        { "hint", "read the 6-digit code from the application window (or its log), then run: aui-remote cli pair {} "
                  "--code <code>"_format(app.pid) },
    };
}

AJson doCompare(const ParsedArgs& args) {
    if (args.positional.size() < 2) {
        throw ArgsError("usage: aui-remote cli compare <a.png> <b.png> [--threshold 0..255]");
    }
    auto load = [](const AString& path) {
        auto image = AImage::fromFile(path);
        if (!image) {
            throw clientError("can't read the image " + path, "check the path and that it is a PNG, JPEG or BMP");
        }
        return image;
    };
    auto a = load(args.positional[0]);
    auto b = load(args.positional[1]);
    const auto diff = compareImages(*a, *b, args.getInt("threshold").valueOr(0));
    AJson result = AJson {
        { "identical", !diff.sizeMismatch && diff.differentPixels == 0 },
        { "sizeMismatch", diff.sizeMismatch },
        { "diffPercent", diff.percent() },
        { "differentPixels", int(diff.differentPixels) },
        { "totalPixels", int(diff.totalPixels) },
    };
    if (diff.sizeMismatch) {
        result["a"] = AJson { { "width", int(a->width()) }, { "height", int(a->height()) } };
        result["b"] = AJson { { "width", int(b->width()) }, { "height", int(b->height()) } };
    }
    return result;
}

AString tail(const APath& file, std::size_t bytes = 2000) {
    std::ifstream in(file.toStdString(), std::ios::binary | std::ios::ate);
    if (!in) {
        return {};
    }
    const auto size = std::size_t(in.tellg());
    in.seekg(size > bytes ? size - bytes : 0);
    std::string result((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return AString::fromUtf8(result);
}

AString shellQuote(const AString& s) {
    AString result = "'";
    for (auto c : s) {
        if (c == '\'') {
            result += "'\\''";
        } else {
            result += c;
        }
    }
    return result + "'";
}

AJson doLaunch(const ParsedArgs& args, const Directories& directories) {
    if (args.rest.empty()) {
        throw ArgsError("usage: aui-remote cli launch [--cwd dir] [--env K=V]... [--timeout seconds] -- <exe> [args]");
    }
    const auto key = KeyStore::generateKey();

    AMap<AString, AString> environment = { { "AUI_REMOTE", "1" }, { "AUI_REMOTE_KEY", key } };
    for (const auto& kv : args.getAll("env")) {
        auto eq = kv.find('=');
        if (eq == AString::NPOS || eq == 0) {
            throw ArgsError("--env expects NAME=VALUE, got '{}'"_format(kv));
        }
        environment[kv.substr(0, eq)] = kv.substr(eq + 1);
    }

    const auto logsFolder = APath::getDefaultPath(APath::TEMP) / "aui-remote";
    logsFolder.makeDirs();
    const auto stamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
    const auto stdoutPath = logsFolder / "launch-{}-stdout.txt"_format(stamp);
    const auto stderrPath = logsFolder / "launch-{}-stderr.txt"_format(stamp);

    const auto& exe = args.rest[0];
    AProcess::ProcessCreationInfo info;
    info.environment = std::move(environment);
    if (auto cwd = args.get("cwd")) {
        info.workDir = *cwd;
    }

#if AUI_PLATFORM_WIN
    AString commandLine = "/S /C \"\"" + exe + "\"";
    for (std::size_t i = 1; i < args.rest.size(); ++i) {
        commandLine += " \"" + args.rest[i] + "\"";
    }
    commandLine += " > \"" + stdoutPath + "\" 2> \"" + stderrPath + "\" < NUL\"";
    info.executable = APath(AString::fromLatin1(std::getenv("COMSPEC") ? std::getenv("COMSPEC") : "cmd.exe"));
    info.args = AProcess::ArgSingleString { std::move(commandLine) };
#else
    // the shell redirects the streams to files and replaces itself with the application, so the pid stays the same
    AStringVector shellArgs = { "-c",
                                "exec \"$0\" \"$@\" > " + shellQuote(stdoutPath) + " 2> " + shellQuote(stderrPath) +
                                    " < /dev/null",
                                exe };
    for (std::size_t i = 1; i < args.rest.size(); ++i) {
        shellArgs << args.rest[i];
    }
    info.executable = "/bin/sh";
    info.args = AProcess::ArgStringList { std::move(shellArgs) };
#endif

    auto process = AProcess::create(std::move(info));
    process->run(ASubProcessExecutionFlags::DETACHED);
    const auto pid = process->getPid();

    const auto timeout = std::chrono::seconds(args.getInt("timeout").valueOr(30));
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (auto app = discovery::find(pid, directories.discovery)) {
            KeyStore(directories.keys).store(*app, key);
            return AJson {
                { "pid", int(pid) },
                { "port", int(app->port) },
                { "stdout", stdoutPath.toStdString() },
                { "stderr", stderrPath.toStdString() },
            };
        }
        if (!discovery::isProcessAlive(pid)) {
            throw clientError(
                "the application exited before the remote agent started",
                "check that it links aui::dev_extras and the executable path is right",
                AJson { { "stderrTail", tail(stderrPath) }, { "stderr", stderrPath.toStdString() } });
        }
        std::this_thread::sleep_for(100ms);
    }
    discovery::terminateProcess(pid);
    throw clientError(
        "the remote agent did not start in {} s"_format(timeout.count()),
        "the application was stopped; no discovery file appeared, does it link aui::dev_extras?",
        AJson { { "pid", int(pid) }, { "stderr", stderrPath.toStdString() } });
}

AJson doStop(const ParsedArgs& args, const Directories& directories) {
    const auto pid = pidArgument(args);
    if (!discovery::isProcessAlive(pid)) {
        throw clientError("no process with pid {}"_format(pid), "run `aui-remote cli list`");
    }
    if (!discovery::terminateProcess(pid)) {
        throw clientError("can't stop the process {}"_format(pid), "stop it manually");
    }
    return AJson { { "stopped", true }, { "pid", int(pid) } };
}

AJson doRemoteCommand(const ParsedArgs& args, const CommandRequest& request, const Directories& directories) {
    auto app = selectApp(args, discovery::list(directories.discovery));
    auto key = KeyStore(directories.keys).load(app);
    if (!key) {
        throw RpcError(
            rpc_code::UNAUTHORIZED, "not paired with {}"_format(app.displayName()),
            AJson { { "hint", "run `aui-remote cli pair {}` and enter the code shown by the application"_format(
                                  app.pid) } });
    }
    return RpcClient(app.port, *key).call(request.method, request.params, request.timeout);
}

}   // namespace

Directories Directories::standard() { return { discovery::defaultDirectory(), KeyStore::defaultDirectory() }; }

AppInfo selectApp(const ParsedArgs& args, const AVector<AppInfo>& apps) {
    auto describeAll = [&] {
        AJson candidates = AJson::Array {};
        for (const auto& app : apps) {
            candidates.push_back(toJson(app, false));
        }
        return AJson { { "candidates", std::move(candidates) } };
    };
    if (auto pid = args.get("pid")) {
        auto n = pid->toUInt();
        if (!n) {
            throw ArgsError("--pid must be a number, got '{}'"_format(*pid));
        }
        for (const auto& app : apps) {
            if (app.pid == *n) {
                return app;
            }
        }
        throw clientError("no running application with a remote agent has pid {}"_format(*n),
                          "pick one of data.candidates", describeAll());
    }
    if (apps.empty()) {
        throw clientError("no application with a remote agent is running",
                          "start one with `aui-remote cli launch -- <exe>` or run it with AUI_REMOTE=1");
    }
    if (apps.size() > 1) {
        throw clientError("several applications are running, choose one with --pid N",
                          "pick one of data.candidates", describeAll());
    }
    return apps.first();
}

int run(const AStringVector& args, const Directories& directories) {
    try {
        auto parsed = ParsedArgs::parse(args);
        if (parsed.command.empty() || parsed.command == "help" || parsed.has("help")) {
            note("commands: " + commandNames().join(' '));
            print(AJson { { "commands", [] {
                              AJson r = AJson::Array {};
                              for (const auto& c : commandNames()) {
                                  r.push_back(c);
                              }
                              return r;
                          }() } });
            return parsed.command.empty() ? 2 : 0;
        }
        AJson result;
        if (parsed.command == "list") {
            result = doList(directories);
        } else if (parsed.command == "pair") {
            result = doPair(parsed, directories);
        } else if (parsed.command == "launch") {
            result = doLaunch(parsed, directories);
        } else if (parsed.command == "stop") {
            result = doStop(parsed, directories);
        } else if (parsed.command == "compare") {
            result = doCompare(parsed);
        } else if (auto request = buildRequest(parsed)) {
            result = doRemoteCommand(parsed, *request, directories);
        } else {
            throw ArgsError("unknown command '{}'; commands: {}"_format(parsed.command, commandNames().join(' ')));
        }
        print(result);
        return 0;
    } catch (const ArgsError& e) {
        print(AJson { { "error", RpcError(-32602, e.getMessage(), AJson { { "hint", "see the usage in the message" } })
                                     .toJson() } });
        return 2;
    } catch (const RpcError& e) {
        print(AJson { { "error", e.toJson() } });
        return 1;
    } catch (const AException& e) {
        print(AJson { { "error", RpcError(rpc_code::CLIENT_ERROR, e.getMessage()).toJson() } });
        return 1;
    }
}

AOptional<int> tryRun(const AStringVector& args) {
    if (args.size() < 2 || args[1] != "cli") {
        return std::nullopt;
    }
    return run(AStringVector(args.begin() + 2, args.end()));
}

}   // namespace aui_remote::cli
