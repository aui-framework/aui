#include <range/v3/all.hpp>

#include "AppropriatePortablePackagePredicate.h"

bool aui::updater::AppropriatePortablePackagePredicate::operator()(const AString& packageName) const noexcept {
    const auto& q = getQualifier();
    if (!packageName.endsWith(".zip")) {
        return false;
    }

    if (auto p = packageName.find(q); p != std::string::npos) {
        // check for _
        p += q.length();
        if (p >= packageName.length()) {
            return true;
        }
        return packageName[p] != '_';
    }

    return false;
}

const AString& aui::updater::AppropriatePortablePackagePredicate::getQualifier() const {
    if (!qualifier.empty()) {
        return qualifier;
    }
    static const AString defaultQualifier = AUI_DEFAULT_PACKAGE_QUALIFIER;
    return defaultQualifier;
}

AString aui::updater::AppropriatePortablePackagePredicate::getQualifierDebug() const {
    return getQualifier() + ", zip";
}
