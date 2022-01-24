#pragma once


#include "AEmbedAuiWrap.h"

class AGLEmbedAuiWrap: public AEmbedAuiWrap {
public:
    AGLEmbedAuiWrap();
    void render();
    void resetGLState();

};


