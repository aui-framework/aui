#include "KeyStore.h"

#include <cstdio>
#include <fstream>
#include <random>
#include <set>
#include <sstream>

namespace aui_remote {

namespace {

AString sessionFileNameFor(const AppInfo& app) {
    std::string startedAt = app.startedAt.toStdString();
    for (auto& c : startedAt) {
        if (!std::isalnum(static_cast<unsigned char>(c))) {
            c = '_';
        }
    }
    return "{}-{}.session"_format(app.pid, startedAt);
}

AString fileNameFor(const AppInfo& app) {
    std::string exe = app.exe.toStdString();
    std::uint64_t hash = 14695981039346656037ull;   // FNV-1a, the name only has to be stable
    for (unsigned char c : exe) {
        hash = (hash ^ c) * 1099511628211ull;
    }
    std::string name = APath(app.exe).filename().toStdString();
    for (auto& c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '.' && c != '-') {
            c = '_';
        }
    }
    char suffix[17];
    std::snprintf(suffix, sizeof(suffix), "%016llx", static_cast<unsigned long long>(hash));
    return "{}-{}.key"_format(name, suffix);
}

void writePrivate(const APath& path, const AString& key) {
    path.parent().makeDirs();
    // the file is restricted before the key gets into it
    { std::ofstream(path.toStdString(), std::ios::binary | std::ios::trunc); }
    path.chmod(0600);
    std::ofstream out(path.toStdString(), std::ios::binary | std::ios::trunc);
    out << key.toStdString();
    if (!out) {
        throw AException("can't write {}"_format(path));
    }
}

AOptional<AString> readKey(const APath& path) {
    std::ifstream in(path.toStdString(), std::ios::binary);
    if (!in) {
        return std::nullopt;
    }
    std::stringstream ss;
    ss << in.rdbuf();
    const auto text = AString::fromUtf8(ss.str());
    AString key(text.trim());
    if (key.empty()) {
        return std::nullopt;
    }
    return key;
}

}   // namespace

KeyStore::KeyStore(APath directory) : mDirectory(std::move(directory)) {}

APath KeyStore::defaultDirectory() { return discovery::defaultDirectory() / "clients"; }

AString KeyStore::generateKey() {
    std::random_device device;
    std::string key;
    constexpr char DIGITS[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        key += DIGITS[device() % 16];
    }
    return key;
}

APath KeyStore::pathFor(const AppInfo& app) const { return mDirectory / fileNameFor(app); }

APath KeyStore::sessionPathFor(const AppInfo& app) const { return mDirectory / sessionFileNameFor(app); }

AOptional<AString> KeyStore::load(const AppInfo& app) const {
    if (auto session = readKey(sessionPathFor(app))) {
        return session;
    }
    return readKey(pathFor(app));
}

void KeyStore::store(const AppInfo& app, const AString& key) const { writePrivate(pathFor(app), key); }

void KeyStore::storeSession(const AppInfo& app, const AString& key) const { writePrivate(sessionPathFor(app), key); }

void KeyStore::remove(const AppInfo& app) const {
    for (const auto& path : { pathFor(app), sessionPathFor(app) }) {
        if (path.isRegularFileExists()) {
            path.removeFile();
        }
    }
}

void KeyStore::pruneExcept(const AVector<AppInfo>& alive) const {
    if (!mDirectory.isDirectoryExists()) {
        return;
    }
    std::set<std::string> keep;
    for (const auto& app : alive) {
        keep.insert(sessionFileNameFor(app).toStdString());
    }
    for (const auto& file : mDirectory.listDir(AFileListFlags::DEFAULT_FLAGS)) {
        if (file.extension() == "session" && !keep.contains(file.filename().toStdString())) {
            file.removeFile();
        }
    }
}

}   // namespace aui_remote
