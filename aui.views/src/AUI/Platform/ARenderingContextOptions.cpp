//
// Created by Alex2772 on 6/17/2022.
//

#include "ARenderingContextOptions.h"

ARenderingContextOptions& ARenderingContextOptions::inst() {
    static ARenderingContextOptions o = {
        {
            OpenGL {},
            Software {},
        },
    };
    return o;
}
