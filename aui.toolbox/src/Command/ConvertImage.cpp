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

#include "ConvertImage.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/Image/png/PngImageLoader.h"
#include "AUI/Image/bmp/BmpImageLoader.h"
#include <AUI/Image/svg/SvgImageFactory.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Traits/strings.h>

AString ConvertImage::getName() {
    return "convert-image";
}

AString ConvertImage::getSignature() {
    return "<input file> <output file> [-c=<canvas size>] [-p=<0x0 position on canvas>] [-r=<256x256 resize>] [-b=<#000 background color>]";
}

static glm::ivec2 contain(glm::ivec2 canvasSize, glm::ivec2 imageSize) {
    if (canvasSize.x == 0 || canvasSize.y == 0) {
        return {};
    }

    glm::ivec2 sizeDelta = canvasSize - imageSize;
    glm::ivec2 size;
    if (canvasSize.x * imageSize.y / canvasSize.y < imageSize.x) {
        size.x = canvasSize.x;
        size.y = size.x * imageSize.y / imageSize.x;
    } else {
        size.y = canvasSize.y;
        size.x = size.y * imageSize.x / imageSize.y;
    }
    return size;
}


static glm::ivec2 parseSize(const AString& value) {
    auto s = value.split('x');
    if (s.size() == 2) {
        return glm::ivec2(s[0].toIntOrException(), s[1].toIntOrException());
    }
    if (s.size() == 1) {
        return glm::ivec2(s[0].toIntOrException());
    }
    return {};
}

AString ConvertImage::getDescription() {
    return "produces image(s) from svg image. Target format is determined by output file extension\n";
}

void ConvertImage::run(Toolbox& t) {
    if (t.args.size() < 2) {
        throw IllegalArgumentsException("convert-image requires at least two arguments");
    }
    APath input;
    APath output;
    glm::ivec2 canvasSize{};
    AOptional<glm::ivec2> position{};
    AOptional<glm::ivec2> resize{};
    AOptional<AColor> backgroundColor;
    for (auto& f : t.args) {
        if (f.length() >= 3) {
            if (f[2] == '=') {
                auto value = f.substr(3);
                switch (f[1]) {
                    case 'c': {
                        canvasSize = parseSize(value);
                        break;
                    }
                    case 'p': {
                        position = parseSize(value);
                        break;
                    }
                    case 'r': {
                        resize = parseSize(value);
                        break;
                    }
                    case 'b': {
                        backgroundColor = AColor(value);
                        break;
                    }
                }
                continue;
            }
        }
        if (input.empty()) {
            input = std::move(f);
            continue;
        }
        if (output.empty()) {
            output = std::move(f);
            continue;
        }
        throw IllegalArgumentsException("unexpected argument {}"_format(f));
    }


    SvgImageFactory img(AByteBuffer::fromStream(AFileInputStream(input)));
    if (canvasSize == glm::ivec2(0)) {
        canvasSize = img.getSizeHint();
    }
    AFormattedImage<APixelFormat::RGBA_BYTE> result(canvasSize);
    result.fill(AFormattedColor<APixelFormat::RGBA_BYTE>{0, 0, 0, 0});

    // contain
    auto imageSize = resize.valueOr(contain(canvasSize, img.getSizeHint()));
    result.insert(position.valueOr((canvasSize - imageSize) / 2), img.provideImage(imageSize));

    if (backgroundColor) {
        // process transparency
        for (auto& color : result) {
            auto src = AColor(color);
            color = AFormattedColorConverter(AColor(glm::vec4(glm::mix(glm::vec3(*backgroundColor), glm::vec3(src), src.a), 1.f)));
        }
    }

    AFileOutputStream fos(output);
    if (output.extension() == "png") {
        PngImageLoader::save(fos, result);
        return;
    }
    if (output.extension() == "bmp") {
        BmpImageLoader::save(fos, result);
        PngImageLoader::save(fos, result);
        return;
    }
}
