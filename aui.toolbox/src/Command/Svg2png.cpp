// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
                        outputDir = std::move(value);
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
