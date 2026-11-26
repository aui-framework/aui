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

#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/ARaiiHelper.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Platform/emscripten/JsPromise.h>

#include <emscripten.h>

#include <tuple>

glm::ivec2 ADesktop::getMousePosition() {
    return {};
}

void ADesktop::setMousePos(const glm::ivec2 &pos) {

}

AFuture<APath> ADesktop::browseForDir(ASurface* parent,
                                      const APath& startingLocation)  {
    return AFuture<APath>::fromValue(APath());
}

namespace {
// clang-format off
// the browser's file picker: resolves with [name, bytes], or ['', empty] if the user closed it without a file
EM_JS(void, aui_desktop_pick_file, (int id, const char* accept), {
    const types = UTF8ToString(accept);
    Module.auiSettle(id, (signal) => new Promise((resolve, reject) => {
        const input = document.createElement('input');
        input.type = 'file';
        if (types) input.accept = types;
        input.style.display = 'none';
        document.body.appendChild(input);
        const nothing = () => { input.remove(); resolve(['', new ArrayBuffer(0)]); };
        input.addEventListener('change', () => {
            const file = input.files && input.files[0];
            if (!file) { nothing(); return; }
            file.arrayBuffer().then((bytes) => { input.remove(); resolve([file.name, bytes]); },
                                    (error) => { input.remove(); reject(error); });
        });
        input.addEventListener('cancel', nothing);   // not every browser tells: then the future just never completes
        signal.addEventListener('abort', nothing);
        input.click();
    }));
});

// a download of `size` bytes at `data` as the file `name`
EM_JS(void, aui_desktop_download, (const char* name, const char* data, size_t size), {
    const url = URL.createObjectURL(new Blob([HEAPU8.slice(data, data + size)], { type: 'application/octet-stream' }));
    const link = document.createElement('a');
    link.href = url;
    link.download = UTF8ToString(name);
    link.style.display = 'none';
    document.body.appendChild(link);
    link.click();
    link.remove();
    setTimeout(() => URL.revokeObjectURL(url), 60000);
});
// clang-format on

/// `accept` of an `<input type=file>`: ".txt,.png"; empty (any file) if one of them is "*".
std::string acceptOf(const AVector<ADesktop::FileExtension>& extensions) {
    std::string accept;
    for (const auto& ext : extensions) {
        if (ext.extension == "*") {
            return {};
        }
        if (!accept.empty()) {
            accept += ',';
        }
        accept += "." + ext.extension.toStdString();
    }
    return accept;
}
}   // namespace

AFuture<APath>
ADesktop::browseForFile(ASurface* parent,
                        const APath& startingLocation,
                        const AVector<FileExtension>& extensions) {
    const auto accept = acceptOf(extensions);
    return aui::js::promise<std::tuple<AString, AByteBuffer>>([&](int id) { aui_desktop_pick_file(id, accept.c_str()); })
        .then([](const std::tuple<AString, AByteBuffer>& picked) {
            const auto& [name, bytes] = picked;
            if (name.empty()) {
                return APath();
            }
            APath dir = "/tmp/aui-picked";
            dir.makeDirs();
            auto path = dir.file(APath(name).filename());
            AFileOutputStream(path).write(bytes.data(), bytes.size());
            return path;
        });
}

AFuture<APath> ADesktop::browseForSaveFile(ASurface* parent,
                                           const APath& startingLocation,
                                           const AString& suggestedName,
                                           const AVector<FileExtension>& extensions) {
    return AFuture<APath>::fromValue(APath());   // a page can't choose where to write: see saveFile
}

AFuture<APath> ADesktop::saveFile(ASurface* parent,
                                  const AString& suggestedName,
                                  AByteBuffer contents,
                                  const AVector<FileExtension>& extensions) {
    aui_desktop_download(suggestedName.toStdString().c_str(), contents.data(), contents.size());
    return AFuture<APath>::fromValue(APath(suggestedName));
}


_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    return nullptr;
}

void ADesktop::playSystemSound(ADesktop::SystemSound s)
{
    // unsupported
}