/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ASingleInstance.h"
#include <AUI/Common/AException.h>
#include <AUI/Platform/Entry.h>
#include <cstdint>
#include <cstdlib>

namespace {
constexpr uint32_t MAGIC = 0x41554931;   // "AUI1"
constexpr uint32_t MAX_STRING_SIZE = 1024 * 1024;

void writeU32(std::string& out, uint32_t v) {
    for (int i = 0; i < 4; ++i) {
        out.push_back(char((v >> (i * 8)) & 0xff));
    }
}

void writeString(std::string& out, const AString& s) {
    const auto& bytes = s.toStdString();
    writeU32(out, uint32_t(bytes.size()));
    out += bytes;
}

struct Reader {
    std::string_view data;

    uint32_t u32() {
        if (data.size() < 4) {
            throw AException("ASingleInstance: truncated message");
        }
        uint32_t v = 0;
        for (int i = 0; i < 4; ++i) {
            v |= uint32_t(uint8_t(data[i])) << (i * 8);
        }
        data.remove_prefix(4);
        return v;
    }

    AString string() {
        auto size = u32();
        if (size > MAX_STRING_SIZE || data.size() < size) {
            throw AException("ASingleInstance: malformed string");
        }
        AString result(data.substr(0, size));
        data.remove_prefix(size);
        return result;
    }
};
}   // namespace

std::string aui::detail::single_instance::serialize(const AActivation& activation) {
    std::string result;
    writeU32(result, MAGIC);
    writeU32(result, uint32_t(activation.args.size()));
    for (const auto& arg : activation.args) {
        writeString(result, arg);
    }
    writeString(result, activation.workingDir);
    writeString(result, activation.activationToken);
    return result;
}

AActivation aui::detail::single_instance::deserialize(std::string_view data) {
    Reader r { data };
    if (r.u32() != MAGIC) {
        throw AException("ASingleInstance: bad magic");
    }
    AActivation result;
    auto argc = r.u32();
    if (argc > 4096) {
        throw AException("ASingleInstance: too many args");
    }
    for (uint32_t i = 0; i < argc; ++i) {
        result.args << r.string();
    }
    result.workingDir = r.string();
    result.activationToken = r.string();
    return result;
}

AActivation aui::detail::single_instance::makeCurrentActivation() {
    AActivation result {
        .args = aui::args(),
        .workingDir = APath::workingDir(),
    };
#if AUI_PLATFORM_LINUX
    for (const char* env : { "XDG_ACTIVATION_TOKEN", "DESKTOP_STARTUP_ID" }) {
        if (auto v = std::getenv(env); v && *v) {
            result.activationToken = v;
            break;
        }
    }
#endif
    return result;
}

AString aui::detail::single_instance::sanitizeKey(const AString& key) {
    AString result;
    for (auto c : key) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-' ||
            c == '_') {
            result += c;
        } else {
            result += '_';
        }
    }
    if (result.empty()) {
        throw AException("ASingleInstance: empty key");
    }
    return result;
}
