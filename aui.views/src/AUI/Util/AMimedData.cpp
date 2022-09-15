//
// Created by Alex2772 on 9/15/2022.
//

#include "AMimedData.h"

void AMimedData::setText(const AString& text) {
    setData("text/plain", AByteBuffer::fromString(text));
}
