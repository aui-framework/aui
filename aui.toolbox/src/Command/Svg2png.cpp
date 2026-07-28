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

//
// Created by Alex2772 on 2/8/2022.
//

#include "Svg2png.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Image/png/PngImageLoader.h"
#include "AUI/IO/AFileOutputStream.h"
#include <AUI/Image/svg/SvgImageFactory.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Traits/strings.h>

AString Svg2png::getName() {
    return "svg2png";
}

AString Svg2png::getSignature() {
    return "<input file> [-r=<size1[,size2[,...]]>] [-o=<output-dir>] [-p=<prefix>]";
}

AString Svg2png::getDescription() {
    return "produces png image(s) from svg image.\n"
           "\t-r image size(s)\n"
           "\t-o output dir\n"
           "\t-p file name prefix\n"
           ;
}

void Svg2png::run(Toolbox& t) {
    if (t.args.empty()) {
        throw IllegalArgumentsException("svg2png requires at least one argument");
    }
    APath file;
    AVector<int> resolutions;
    APath outputDir = ".";
    AString filenamePrefix = "output";
    for (auto& f : t.args) {
        if (f.length() >= 3) {
            if (f[2] == '=') {
                auto value = f.substr(3);
                switch (f[1]) {
                    case 'r': {
                        for (auto& resolution: value.split(',')) {
                            resolutions << resolution.toInt().valueOr(0);
                        }
                        break;
                    }
                    case 'o': {
                        outputDir = APath(value);
                        break;
                    }
                    case 'p': {
                        filenamePrefix = std::move(value);
                        break;
                    }
                }
                continue;
            }
        }
        file = f;
    }
    if (file.empty()) {
        throw IllegalArgumentsException("input file does not set");
    }
    SvgImageFactory img(AByteBuffer::fromStream(AFileInputStream(file)));
    if (resolutions.empty()) {
        resolutions << img.getSizeHint().x;
    }
    auto filename = file.filename();
    auto filenameNoExt = file.filenameWithoutExtension();

    outputDir.makeDirs();
    for (auto& r : resolutions) {
        auto outputFilename = "{}_{}x{}.png"_format(filenamePrefix, r, r);
        AFileOutputStream fos(outputDir / outputFilename);
        PngImageLoader::save(fos, img.provideImage({r, r}));
        std::cout << filename << " -> " << outputFilename << std::endl;
    }
}
