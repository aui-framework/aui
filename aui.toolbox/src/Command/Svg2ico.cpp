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
