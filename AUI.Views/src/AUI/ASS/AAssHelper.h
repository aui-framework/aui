//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Common/AVector.h>

#include "Declaration/BackgroundImage.h"

class Rule;

/**
 * \brief И помни, ASS - это не жопа. ASS - это Aui Style Sheets
 */
class AAssHelper: public AObject {
    friend class AView;

private:
    AVector<const Rule*> mPossiblyApplicableRules;

public:
    void onInvalidateFullAss() {
        emit invalidateFullAss();
    }
    void onInvalidateStateAss() {
        emit invalidateStateAss();
    }

    struct {
        ass::BackgroundImage backgroundUrl;
    } state;
signals:
    emits<> invalidateFullAss;
    emits<> invalidateStateAss;
};



