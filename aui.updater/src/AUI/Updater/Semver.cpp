#include <AUI/Util/ATokenizer.h>
#include "Semver.h"

aui::updater::Semver aui::updater::Semver::fromString(const AString& version) {
    Semver out;
    try {
        ATokenizer p(version);
        if (auto c = p.readChar(); c != 'v') {
            p.reverseByte();
        }
        for (auto i : { &out.major, &out.minor, &out.patch }) {
            *i = p.readUInt();
            if (auto c = p.readChar(); c != '.') {
                return out;
            }
        }
    } catch (const AEOFException&) {}
    return out;
}

bool aui::updater::Semver::operator<(const aui::updater::Semver& rhs) const {
    if (major < rhs.major)
        return true;
    if (rhs.major < major)
        return false;
    if (minor < rhs.minor)
        return true;
    if (rhs.minor < minor)
        return false;
    return patch < rhs.patch;
}
bool aui::updater::Semver::operator>(const aui::updater::Semver& rhs) const { return rhs < *this; }
bool aui::updater::Semver::operator<=(const aui::updater::Semver& rhs) const { return !(rhs < *this); }
bool aui::updater::Semver::operator>=(const aui::updater::Semver& rhs) const { return !(*this < rhs); }
