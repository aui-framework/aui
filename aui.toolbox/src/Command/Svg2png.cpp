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
    return "<input file> [-r<size1[,size2[,...]]>]";
}

AString Svg2png::getDescription() {
    return "produces png image(s) from svg image.\n"
           "\t-r specifies image size(s)";
}

void Svg2png::run(Toolbox& t) {
    if (t.args.empty()) {
        throw IllegalArgumentsException("svg2png requires at least one argument");
    }
    APath file;
    AVector<int> resolutions;
    for (auto& f : t.args) {
        if (f.startsWith("-")) {
            if (f.startsWith("-r")) {
                auto sResolutions = f.mid(2);
                for (auto& resolution : sResolutions.split(',')) {
                    resolutions << resolution.toInt();
                }
            }
        } else {
            file = f;
        }
    }
    SvgImageFactory img(AByteBuffer::fromStream(AFileInputStream(file)));
    if (resolutions.empty()) {
        resolutions << img.getSizeHint().x;
    }
    auto filename = file.filename();
    auto filenameNoExt = file.filenameWithoutExtension();
    for (auto& r : resolutions) {
        auto outputFilename = "{}_{}.png"_format(filenameNoExt, r);
        AFileOutputStream fos(outputFilename);
        PngImageLoader::save(fos, *img.provideImage({r, r}));
        std::cout << filename << " -> " << outputFilename << std::endl;
    }
}
