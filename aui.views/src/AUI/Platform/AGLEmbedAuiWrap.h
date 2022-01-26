#pragma once


#include "AEmbedAuiWrap.h"

class API_AUI_VIEWS AGLEmbedAuiWrap: public AEmbedAuiWrap {
public:
    AGLEmbedAuiWrap();
    void render();
    void resetGLState();

};


