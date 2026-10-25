#include "KeyStore.h"

#include <fstream>
#include <random>
#include <set>
#include <sstream>

namespace aui_remote {

namespace {

AString fileNameFor(const AppInfo& app) {
    std::string startedAt = app.startedAt.toStdString();
    for (auto& c : startedAt) {
        if (!std::isalnum(static_cast<unsigned char>(c))) {
            c = '_';
        }
    }
    return "{}-{}.key"_format(app.pid, startedAt);
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

AOptional<AString> KeyStore::load(const AppInfo& app) const {
    std::ifstream in(pathFor(app).toStdString(), std::ios::binary);
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

void KeyStore::store(const AppInfo& app, const AString& key) const {
    mDirectory.makeDirs();
    const auto path = pathFor(app);
    // the file is restricted before the key gets into it
    { std::ofstream(path.toStdString(), std::ios::binary | std::ios::trunc); }
    path.chmod(0600);
    std::ofstream out(path.toStdString(), std::ios::binary | std::ios::trunc);
    out << key.toStdString();
    if (!out) {
        throw AException("can't write {}"_format(path));
    }
}

void KeyStore::remove(const AppInfo& app) const {
    if (auto path = pathFor(app); path.isRegularFileExists()) {
        path.removeFile();
    }
}

void KeyStore::pruneExcept(const AVector<AppInfo>& alive) const {
    if (!mDirectory.isDirectoryExists()) {
        return;
    }
    std::set<std::string> keep;
    for (const auto& app : alive) {
        keep.insert(fileNameFor(app).toStdString());
    }
    for (const auto& file : mDirectory.listDir(AFileListFlags::DEFAULT_FLAGS)) {
        if (file.extension() == "key" && !keep.contains(file.filename().toStdString())) {
            file.removeFile();
        }
    }
}

}   // namespace aui_remote
