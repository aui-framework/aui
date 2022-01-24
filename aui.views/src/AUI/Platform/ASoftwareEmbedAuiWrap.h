#pragma once


#include "AEmbedAuiWrap.h"
#include "SoftwareRenderingContext.h"

class ASoftwareEmbedAuiWrap: public AEmbedAuiWrap {
private:
    SoftwareRenderingContext* mContext;

public:
    ASoftwareEmbedAuiWrap();
    AImage render();

};


