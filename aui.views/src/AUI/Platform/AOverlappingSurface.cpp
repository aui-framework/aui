//
// Created by Alex2772 on 11/13/2021.
//

#include "AOverlappingSurface.h"
#include "ABaseWindow.h"


void AOverlappingSurface::close() {
    mWindow->closeOverlappingSurface(this);
}
