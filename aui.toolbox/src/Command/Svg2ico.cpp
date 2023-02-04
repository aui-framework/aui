// AUI Framework - Declarative UI toolkit for modern C++20
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

#include "Svg2ico.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Image/png/PngImageLoader.h"
#include "AUI/IO/AFileOutputStream.h"
#include <AUI/Image/svg/SvgImageFactory.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Traits/strings.h>
#include <AUI/Image/ico/IcoImageLoader.h>

AString Svg2ico::getName() {
    return "svg2ico";
}

AString Svg2ico::getSignature() {
    return "<input file> <output file>";
}

AString Svg2ico::getDescription() {
    return "produces ico image(s) from svg image.";
}

void Svg2ico::run(Toolbox& t) {
    if (t.args.size() != 2) {
        throw IllegalArgumentsException("svg2png requires 2 arguments");
    }

    auto& inputFile = t.args[0];
    auto& outputFile = t.args[1];

    SvgImageFactory img(AByteBuffer::fromStream(AFileInputStream(inputFile)));

    AVector<AImage> images;
    for (auto size : { 16, 32, 48, 64, 128, 256}) {
        images << std::move(img.provideImage({size, size}));
    }
    IcoImageLoader::save(AFileOutputStream(outputFile), images);
    std::cout << inputFile << " -> " << outputFile << std::endl;
}
