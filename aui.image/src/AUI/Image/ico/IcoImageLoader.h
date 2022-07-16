#pragma once


#include <AUI/Image/AImage.h>

class API_AUI_IMAGE IcoImageLoader {
public:

    static void save(aui::no_escape<IOutputStream> os, const AVector<AImage>& images);
};


