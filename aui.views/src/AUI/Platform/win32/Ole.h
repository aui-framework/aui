#pragma once


#include "AUI/Util/AMimedData.h"

class IDataObject;

class Ole {
public:
    ~Ole();

    static Ole& inst();

    static AMimedData toMime(IDataObject* dataObject);
    static IDataObject* fromMime(const AMimedData& data);

private:
    Ole();
};


