#pragma once


#include "AEmbedAuiWrap.h"
#include "SoftwareRenderingContext.h"

class API_AUI_VIEWS ASoftwareEmbedAuiWrap: public AEmbedAuiWrap {
private:
    SoftwareRenderingContext* mContext;

public:
    ASoftwareEmbedAuiWrap();
    AImage render();

};


